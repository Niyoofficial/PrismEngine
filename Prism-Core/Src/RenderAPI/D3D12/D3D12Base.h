#pragma once

#include "D3D12Utils.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <wrl/client.h>
#include <corecrt_math_defines.h>

#ifdef PE_BUILD_DEBUG
#include <crtdbg.h>
#endif

#include <d3d12.h>
#include <dxgi1_6.h>
#ifdef PE_BUILD_DEBUG
#include <dxgidebug.h>
#endif

#include "d3dx12/d3dx12.h"

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;
namespace DirectX {}
namespace DX = ::DirectX;
