#pragma once

#include <cstdint>
#include <vector>
#include <string>

static int32_t GNames = 0x18814B40;
static int32_t NamePrivate = 0x20;
class FName
{
public:
    int32_t ComparisonIndex;
 
    FName()
    {
        ComparisonIndex = 0;
    }
 
    FName(uintptr_t object)
    {
        if (object)
        {
            ComparisonIndex = DotMem::Read<int32_t>(object + Offsets::NamePrivate);
        }
    }
 
    std::string ToString() const
    {
        return ToString(ComparisonIndex);
    }
 
    static std::string ToString(uintptr_t object)
    {
        return FName(object).ToString();
    }
 
    static std::string ToString(int32_t index)
    {
        uint32_t BlockCount = DotMem::Read<uint32_t>(DotMem::BaseAddress + Offsets::GNames) + 1;
        auto GetEntry = [&](uint32_t index) -> uint64_t
        {
            uint32_t BlockIndex = index >> 16;
            if (BlockIndex >= BlockCount) return 0;
 
            uint64_t Block = DotMem::Read<uint64_t>(DotMem::BaseAddress + Offsets::GNames + 8 + (BlockIndex * 8));
            if (!Block) return 0;
 
            return Block + 2 * uint16_t(index);
        };
 
        int32_t Index = DecryptIndex(index);
        uint64_t Entry = GetEntry(Index);
        if (!Entry) return "";
 
        uint16_t Header = DotMem::Read<uint16_t>(Entry);
        uint32_t RawLength = (Header >> 5) & 0x3FF;
 
        if (RawLength == 0x383)
        {
            Index = DecryptIndex(DotMem::Read<int32_t>(Entry + 6));
            Entry = GetEntry(Index);
            if (!Entry) return "";
 
            Header = DotMem::Read<uint16_t>(Entry);
            RawLength = (Header >> 5) & 0x3FF;
        }
 
        int32_t Length = static_cast<int32_t>(RawLength ^ 0x383);
        if (Length <= 0 || Length >= 1024) return "";
 
        const bool IsWide = (Header & 0x8000u) != 0;
        const uint32_t EncodedBytes = IsWide ? static_cast<uint32_t>(Length) * 2u : static_cast<uint32_t>(Length);
        std::vector<uint8_t> Encoded(EncodedBytes);
        DotMem::ReadMemory(Entry + 2, Encoded.data(), EncodedBytes);
        std::vector<uint8_t> Decoded(EncodedBytes);
        DecryptName(Encoded.data(), Decoded.data(), Length, EncodedBytes);
 
        std::vector<char> Buffer(Length + 1);
        if (IsWide)
        {
            for (int32_t i = 0; i < Length; i++)
            {
                uint8_t lo = Decoded[static_cast<size_t>(i) * 2];
                uint8_t hi = Decoded[static_cast<size_t>(i) * 2 + 1];
                Buffer[i] = hi == 0 ? static_cast<char>(lo) : '?';
            }
        }
        else
        {
            for (int32_t i = 0; i < Length; i++)
                Buffer[i] = static_cast<char>(Decoded[i]);
        }
        Buffer[Length] = '\0';
        int32_t Number = DecryptNumber(DotMem::Read<int32_t>(Entry + 2));
        if (Number >= 0)
            return std::string(Buffer.data()) + "_" + std::to_string(Number);
 
        return std::string(Buffer.data());
    }
 
    static int32_t DecryptIndex(int32_t index)
    {
        if (!index) return 0;
        int32_t Decrypted = ((index - 1) ^ 0x57C9BBE3) + 1;
        return Decrypted ? Decrypted : 0xA836441D;
    }
 
    static int32_t DecryptNumber(int32_t number)
    {
        if (number < 1) return -1;
        int32_t value = number - 1;
        int32_t decrypted = value ^ 0x188EEA43;
        if (value == 0xE77115BC) decrypted = value;
        return decrypted;
    }
 
    static void DecryptName(const uint8_t* input, uint8_t* output, int32_t length, uint32_t encodedBytes)
    {
        uint32_t key = static_cast<uint32_t>(length);
        for (uint32_t i = 0; i < encodedBytes; i++)
        {
            uint8_t mix = static_cast<uint8_t>(key * 0x50u);
            output[i] = static_cast<uint8_t>(~input[i] + mix + 0xB9);
            key = (4294958928 * key) + 0xC92828BC;
        }
    }
};
