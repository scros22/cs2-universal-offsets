// cswapchaindx11.h  -  CS2 build 14183  -  cs2-sdk.com
// The engine's DX11 swap-chain wrapper. CreateSwapChain (this, IDXGIFactory*, device, flags) calls IDXGIFactory::CreateSwapChain (factory vtable slot 10) with &this->m_pSwapChain as the out pointer, then logs 'Successfully created dx11 swap chain %s'. Verified in IDA on build 2000915. Hook IDXGISwapChain::Present / ResizeBuffers through the object stored here.
// Module: rendersystemdx11.dll. Offsets drift between builds - regenerate after a CS2 update.
#pragma once
#include <cstddef>
#include <cstdint>

namespace CSwapChainDx11 {

// one per window; CreateSwapChain receives the instance in rcx - read m_pSwapChain after it returns
inline constexpr std::ptrdiff_t kCreateSwapChain_rva = 0x3E7D0; // pattern CSwapChainDx11_CreateSwapChain

// --- fields ---
inline constexpr std::ptrdiff_t m_pSwapChain = 0x170 ; // IDXGISwapChain* - IDXGISwapChain vtable: 8 Present, 9 GetBuffer, 10 SetFullscreenState, 12 GetDesc, 13 ResizeBuffers, 14 ResizeTarget
} // namespace CSwapChainDx11
