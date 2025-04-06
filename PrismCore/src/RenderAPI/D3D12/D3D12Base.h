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

#include "directx/d3dx12.h"

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;
namespace DirectX {}
namespace DX = ::DirectX;


template<>
struct std::hash<D3D12_CONSTANT_BUFFER_VIEW_DESC>
{
	size_t operator()(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc) const noexcept
	{
		static_assert(sizeof(desc) == 16);

		return
			std::hash<D3D12_GPU_VIRTUAL_ADDRESS>()(desc.BufferLocation) ^
			std::hash<UINT>()(desc.SizeInBytes);
	}
};

template<>
struct std::hash<D3D12_SHADER_RESOURCE_VIEW_DESC>
{
	size_t operator()(const D3D12_SHADER_RESOURCE_VIEW_DESC& desc) const noexcept
	{
		static_assert(sizeof(desc) == 40);

		size_t hash =
			std::hash<DXGI_FORMAT>()(desc.Format) ^
			std::hash<D3D12_SRV_DIMENSION>()(desc.ViewDimension) ^
			std::hash<UINT>()(desc.Shader4ComponentMapping);

		switch (desc.ViewDimension)
		{
		case D3D12_SRV_DIMENSION_BUFFER:
			hash ^=
				std::hash<UINT64>()(desc.Buffer.FirstElement) ^
				std::hash<UINT>()(desc.Buffer.NumElements) ^
				std::hash<UINT>()(desc.Buffer.StructureByteStride) ^
				std::hash<UINT>()(desc.Buffer.Flags);
			break;
		case D3D12_SRV_DIMENSION_TEXTURE1D:
			hash ^=
				std::hash<UINT>()(desc.Texture1D.MostDetailedMip) ^
				std::hash<UINT>()(desc.Texture1D.MipLevels) ^
				std::hash<FLOAT>()(desc.Texture1D.ResourceMinLODClamp);
			break;
		case D3D12_SRV_DIMENSION_TEXTURE1DARRAY:
			hash ^=
				std::hash<UINT>()(desc.Texture1DArray.MostDetailedMip) ^
				std::hash<UINT>()(desc.Texture1DArray.MipLevels) ^
				std::hash<UINT>()(desc.Texture1DArray.FirstArraySlice) ^
				std::hash<UINT>()(desc.Texture1DArray.ArraySize) ^
				std::hash<FLOAT>()(desc.Texture1DArray.ResourceMinLODClamp);
			break;
		case D3D12_SRV_DIMENSION_TEXTURE2D:
			hash ^=
				std::hash<UINT>()(desc.Texture2D.MostDetailedMip) ^
				std::hash<UINT>()(desc.Texture2D.MipLevels) ^
				std::hash<UINT>()(desc.Texture2D.PlaneSlice) ^
				std::hash<FLOAT>()(desc.Texture2D.ResourceMinLODClamp);
			break;
		case D3D12_SRV_DIMENSION_TEXTURE2DARRAY:
			hash ^=
				std::hash<UINT>()(desc.Texture2DArray.MostDetailedMip) ^
				std::hash<UINT>()(desc.Texture2DArray.MipLevels) ^
				std::hash<UINT>()(desc.Texture2DArray.FirstArraySlice) ^
				std::hash<UINT>()(desc.Texture2DArray.ArraySize) ^
				std::hash<UINT>()(desc.Texture2DArray.PlaneSlice) ^
				std::hash<FLOAT>()(desc.Texture2DArray.ResourceMinLODClamp);
			break;
		case D3D12_SRV_DIMENSION_TEXTURE2DMS:
			// No additional fields to hash
			break;
		case D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY:
			hash ^=
				std::hash<UINT>()(desc.Texture2DMSArray.FirstArraySlice) ^
				std::hash<UINT>()(desc.Texture2DMSArray.ArraySize);
			break;
		case D3D12_SRV_DIMENSION_TEXTURE3D:
			hash ^=
				std::hash<UINT>()(desc.Texture3D.MostDetailedMip) ^
				std::hash<UINT>()(desc.Texture3D.MipLevels) ^
				std::hash<FLOAT>()(desc.Texture3D.ResourceMinLODClamp);
			break;
		case D3D12_SRV_DIMENSION_TEXTURECUBE:
			hash ^=
				std::hash<UINT>()(desc.TextureCube.MostDetailedMip) ^
				std::hash<UINT>()(desc.TextureCube.MipLevels) ^
				std::hash<FLOAT>()(desc.TextureCube.ResourceMinLODClamp);
			break;
		case D3D12_SRV_DIMENSION_TEXTURECUBEARRAY:
			hash ^=
				std::hash<UINT>()(desc.TextureCubeArray.MostDetailedMip) ^
				std::hash<UINT>()(desc.TextureCubeArray.MipLevels) ^
				std::hash<UINT>()(desc.TextureCubeArray.First2DArrayFace) ^
				std::hash<UINT>()(desc.TextureCubeArray.NumCubes) ^
				std::hash<FLOAT>()(desc.TextureCubeArray.ResourceMinLODClamp);
			break;
		case D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE:
			hash ^=
				std::hash<D3D12_GPU_VIRTUAL_ADDRESS>()(desc.RaytracingAccelerationStructure.Location);
			break;
		default:
			PE_ASSERT_NO_ENTRY();
		}

		return hash;
	}
};

template<>
struct std::hash<D3D12_UNORDERED_ACCESS_VIEW_DESC>
{
	size_t operator()(const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc) const noexcept
	{
		static_assert(sizeof(desc) == 40);

		size_t hash =
			std::hash<DXGI_FORMAT>()(desc.Format) ^
			std::hash<D3D12_UAV_DIMENSION>()(desc.ViewDimension);

		switch (desc.ViewDimension)
		{
		case D3D12_UAV_DIMENSION_BUFFER:
			hash ^=
				std::hash<UINT64>()(desc.Buffer.FirstElement) ^
				std::hash<UINT>()(desc.Buffer.NumElements) ^
				std::hash<UINT>()(desc.Buffer.StructureByteStride) ^
				std::hash<UINT64>()(desc.Buffer.CounterOffsetInBytes);
			break;
		case D3D12_UAV_DIMENSION_TEXTURE1D:
			hash ^=
				std::hash<UINT>()(desc.Texture1D.MipSlice);
			break;
		case D3D12_UAV_DIMENSION_TEXTURE1DARRAY:
			hash ^=
				std::hash<UINT>()(desc.Texture1DArray.MipSlice) ^
				std::hash<UINT>()(desc.Texture1DArray.FirstArraySlice) ^
				std::hash<UINT>()(desc.Texture1DArray.ArraySize);
			break;
		case D3D12_UAV_DIMENSION_TEXTURE2D:
			hash ^=
				std::hash<UINT>()(desc.Texture2D.MipSlice) ^
				std::hash<UINT>()(desc.Texture2D.PlaneSlice);
			break;
		case D3D12_UAV_DIMENSION_TEXTURE2DARRAY:
			hash ^=
				std::hash<UINT>()(desc.Texture2DArray.MipSlice) ^
				std::hash<UINT>()(desc.Texture2DArray.FirstArraySlice) ^
				std::hash<UINT>()(desc.Texture2DArray.ArraySize) ^
				std::hash<UINT>()(desc.Texture2DArray.PlaneSlice);
			break;
		case D3D12_UAV_DIMENSION_TEXTURE3D:
			hash ^=
				std::hash<UINT>()(desc.Texture3D.MipSlice) ^
				std::hash<UINT>()(desc.Texture3D.FirstWSlice) ^
				std::hash<UINT>()(desc.Texture3D.WSize);
			break;
		default:
			PE_ASSERT_NO_ENTRY();
		}

		return hash;
	}
};
