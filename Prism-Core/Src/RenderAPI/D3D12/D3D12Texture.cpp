#include "pcpch.h"
#include "D3D12Texture.h"

#include "ResourceUploadBatch.h"
#include "RenderAPI/D3D12/D3D12RenderDevice.h"
#include "RenderAPI/D3D12/D3D12TypeConversions.h"

#include "WICTextureLoader.h"

namespace Prism::Render::D3D12
{
using namespace Render;

D3D12Texture::D3D12Texture(const TextureDesc& desc, const std::vector<TextureData>& initData, Flags<ResourceStateFlags> initState)
	: m_originalDesc(desc)
{
	if (!initData.empty())
	{
		// TODO: implement
		PE_ASSERT_NO_ENTRY();
	}

	auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC d3d12Desc = GetD3D12ResourceDesc(m_originalDesc);
	D3D12_CLEAR_VALUE d3d12ClearValue = GetD3D12ClearValue(m_originalDesc.optimizedClearValue);
	PE_ASSERT_HR(D3D12RenderDevice::Get().GetD3D12Device()->CreateCommittedResource(
		&heapProps, D3D12_HEAP_FLAG_NONE,
		&d3d12Desc, GetD3D12ResourceStates(initState),
		&d3d12ClearValue,
		IID_PPV_ARGS(&m_resource)));

	PE_ASSERT_HR(m_resource->SetName(m_originalDesc.textureName.c_str()));
}

D3D12Texture::D3D12Texture(std::wstring filepath, bool loadAsCubemap)
{
	auto* d3d12Device = D3D12RenderDevice::Get().GetD3D12Device();
	DX::ResourceUploadBatch batch(d3d12Device);
	batch.Begin();
	PE_ASSERT_HR(DX::CreateWICTextureFromFile(d3d12Device, batch, filepath.c_str(), &m_resource, true));
	batch.End(D3D12RenderDevice::Get().GetD3D12CommandQueue()).wait_for(std::chrono::seconds(0));

	auto resDesc = m_resource->GetDesc();
	PE_ASSERT(!loadAsCubemap || resDesc.DepthOrArraySize == 6, "Cubemaps must have an array size of 6");

	m_originalDesc = D3D12::GetTextureDesc(resDesc, filepath, ResourceUsage::Default, {}, loadAsCubemap);
}

D3D12Texture::D3D12Texture(ID3D12Resource* resource, const std::wstring& name, ResourceUsage usage,
						   ClearValue optimizedClearValue, bool isCubeTexture)
	: m_originalDesc(D3D12::GetTextureDesc(resource->GetDesc(), name, usage, optimizedClearValue, isCubeTexture)),
	  m_resource(resource)
{
	PE_ASSERT_HR(m_resource->SetName(m_originalDesc.textureName.c_str()));
}

TextureDesc D3D12Texture::GetTextureDesc() const
{
	return D3D12::GetTextureDesc(m_resource->GetDesc(), m_originalDesc.textureName, m_originalDesc.usage,
								 m_originalDesc.optimizedClearValue, m_originalDesc.IsCube());
}
}
