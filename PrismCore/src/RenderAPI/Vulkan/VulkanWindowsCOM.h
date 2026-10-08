#pragma once

#ifdef PE_PLATFORM_WINDOWS

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <wrl/client.h>

#include <combaseapi.h>
#include <objbase.h>
#include <unknwn.h>

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

#endif
