#include "pcpch.h"
#include "D3D12Texture.h"

#include "RenderAPI/D3D12/D3D12RenderContext.h"
#include "RenderAPI/D3D12/D3D12RenderDevice.h"
#include "RenderAPI/D3D12/D3D12TypeConversions.h"

namespace Prism::Render::D3D12
{
using namespace Render;

D3D12Texture::D3D12Texture(TextureDesc desc, const std::vector<TextureInitData>& initData)
	: m_originalDesc(std::move(desc))
{
	D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON;
	if (!initData.empty())
	{
		initState = D3D12_RESOURCE_STATE_COPY_DEST;
		// TODO: implement
		PE_ASSERT_NO_ENTRY();
	}

	auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC d3d12Desc = GetD3D12ResourceDesc(m_originalDesc);
	D3D12_CLEAR_VALUE d3d12ClearValue = GetD3D12ClearValue(m_originalDesc.optimizedClearValue);
	PE_ASSERT_HR(D3D12RenderDevice::Get().GetD3D12Device()->CreateCommittedResource(
		&heapProps, D3D12_HEAP_FLAG_NONE,
		&d3d12Desc, initState,
		&d3d12ClearValue,
		IID_PPV_ARGS(&m_resource)));

	PE_ASSERT_HR(m_resource->SetName(m_originalDesc.textureName.c_str()));
}

D3D12Texture::D3D12Texture(ID3D12Resource* resource, TextureDesc desc)
	: m_originalDesc(std::move(desc)), m_resource(resource)
{
	PE_ASSERT_HR(m_resource->SetName(m_originalDesc.textureName.c_str()));
}

TextureDesc D3D12Texture::GetTextureDesc() const
{
	return D3D12::GetTextureDesc(m_resource->GetDesc(), m_originalDesc.textureName, m_originalDesc.usage,
								 m_originalDesc.optimizedClearValue, m_originalDesc.IsCube());
}
}
