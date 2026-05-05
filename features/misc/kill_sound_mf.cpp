// ---------------------------------------------------------------
// kill_sound_mf.cpp -- isolated Media Foundation audio decoder.
//
// Compiled in its own translation unit because mfapi.h / mfidl.h
// drag in IDL/COM macro definitions that conflict with the d3d11
// headers used by the renderer when both end up in the same TU
// (we get a flood of "C3646: 'Format': unknown override specifier"
// out of dxgi/d3d11). Keeping MF here, outside of the kill_sound.h
// inline blob, avoids that collision entirely.
//
// Public surface: a single C++ free function in namespace KillSound.
// Caller owns the returned buffer (HeapAlloc on the process heap).
// ---------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <atomic>
#include <vector>
#include <cstdint>
#include <cstring>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "ole32.lib")

namespace KillSound
{
    // One-time MF init guard. MFStartup is refcounted, so we lazily
    // call it on first decode. We never pair with MFShutdown -- the
    // DLL outlives any meaningful "shutdown" point for MF, and it's
    // a long-lived runtime singleton anyway.
    static std::atomic<bool> g_mfInited{ false };

    uint8_t* DecodeAudioToWav(const wchar_t* widePath, int* outSize)
    {
        if (outSize) *outSize = 0;
        if (!widePath) return nullptr;

        if (!g_mfInited.exchange(true)) {
            CoInitializeEx(nullptr, COINIT_MULTITHREADED);
            HRESULT hrInit = MFStartup(MF_VERSION, MFSTARTUP_LITE);
            if (FAILED(hrInit)) {
                g_mfInited.store(false);
                return nullptr;
            }
        }

        IMFSourceReader* reader = nullptr;
        HRESULT hr = MFCreateSourceReaderFromURL(widePath, nullptr, &reader);
        if (FAILED(hr) || !reader) return nullptr;

        // Negotiate uncompressed PCM 16-bit. MF inserts the right
        // decoder + resampler MFTs automatically.
        IMFMediaType* want = nullptr;
        hr = MFCreateMediaType(&want);
        if (SUCCEEDED(hr)) hr = want->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
        if (SUCCEEDED(hr)) hr = want->SetGUID(MF_MT_SUBTYPE,    MFAudioFormat_PCM);
        if (SUCCEEDED(hr)) hr = reader->SetCurrentMediaType(
            (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, want);
        if (want) want->Release();
        if (FAILED(hr)) { reader->Release(); return nullptr; }

        IMFMediaType* actual = nullptr;
        hr = reader->GetCurrentMediaType(
            (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, &actual);
        if (FAILED(hr) || !actual) { reader->Release(); return nullptr; }

        UINT32 channels = 0, sampleRate = 0, bitsPerSample = 0;
        actual->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS,       &channels);
        actual->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &sampleRate);
        actual->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE,    &bitsPerSample);
        actual->Release();

        if (channels == 0 || sampleRate == 0 || bitsPerSample == 0) {
            reader->Release();
            return nullptr;
        }

        std::vector<uint8_t> pcm;
        pcm.reserve(64 * 1024);
        for (;;) {
            DWORD flags = 0;
            IMFSample* sample = nullptr;
            hr = reader->ReadSample(
                (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0,
                nullptr, &flags, nullptr, &sample);
            if (FAILED(hr)) { if (sample) sample->Release(); break; }
            if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
                if (sample) sample->Release();
                break;
            }
            if (!sample) continue;

            IMFMediaBuffer* mbuf = nullptr;
            if (SUCCEEDED(sample->ConvertToContiguousBuffer(&mbuf)) && mbuf) {
                BYTE* data = nullptr; DWORD dataLen = 0;
                if (SUCCEEDED(mbuf->Lock(&data, nullptr, &dataLen))) {
                    pcm.insert(pcm.end(), data, data + dataLen);
                    mbuf->Unlock();
                }
                mbuf->Release();
            }
            sample->Release();

            // 16 MB hard cap on decoded PCM (sanity).
            if (pcm.size() > 16u * 1024u * 1024u) break;
        }
        reader->Release();

        if (pcm.empty()) return nullptr;

        const uint32_t dataBytes  = (uint32_t)pcm.size();
        const uint16_t blockAlign = (uint16_t)((channels * bitsPerSample) / 8);
        const uint32_t byteRate   = sampleRate * blockAlign;
        const uint32_t totalSize  = 44 + dataBytes;

        uint8_t* out = (uint8_t*)HeapAlloc(GetProcessHeap(), 0, totalSize);
        if (!out) return nullptr;

        memcpy(out + 0,  "RIFF", 4);
        *(uint32_t*)(out + 4) = totalSize - 8;
        memcpy(out + 8,  "WAVE", 4);
        memcpy(out + 12, "fmt ", 4);
        *(uint32_t*)(out + 16) = 16;                    // fmt chunk size
        *(uint16_t*)(out + 20) = 1;                     // PCM
        *(uint16_t*)(out + 22) = (uint16_t)channels;
        *(uint32_t*)(out + 24) = sampleRate;
        *(uint32_t*)(out + 28) = byteRate;
        *(uint16_t*)(out + 32) = blockAlign;
        *(uint16_t*)(out + 34) = (uint16_t)bitsPerSample;
        memcpy(out + 36, "data", 4);
        *(uint32_t*)(out + 40) = dataBytes;
        memcpy(out + 44, pcm.data(), dataBytes);

        if (outSize) *outSize = (int)totalSize;
        return out;
    }
}
