#include "pcpch.h"
#include "D3D12TextureView.h"

#include "RenderAPI/D3D12/D3D12RenderContext.h"
#include "RenderAPI/D3D12/D3D12RenderDevice.h"
#include "RenderAPI/D3D12/D3D12Texture.h"
#include "RenderAPI/D3D12/D3D12TypeConversions.h"

namespace Prism::Render::D3D12
{
D3D12TextureView::D3D12TextureView(TextureViewDesc desc, Texture* texture)
	: m_viewDesc(desc)
{
	m_owningTexture = texture;

	if (m_viewDesc.format == TextureFormat::Unknown)
		m_viewDesc.format = texture->GetTextureDesc().format;

	m_descriptor = D3D12RenderDevice::Get().AllocateCPUDescriptors(GetD3D12DescriptorHeapType(m_viewDesc.type));

	switch (m_viewDesc.type)
	{
	case TextureViewType::SRV:
		{
			auto d3d12ViewDesc = GetD3D12ShaderResourceViewDesc(m_viewDesc);

			D3D12RenderDevice::Get().GetD3D12Device()->CreateShaderResourceView(
				static_cast<D3D12Texture*>(m_owningTexture.Raw())->GetD3D12Resource(),
				&d3d12ViewDesc, m_descriptor.GetCPUHandle());
		}
		break;
	case TextureViewType::UAV:
		{
			PE_ASSERT_NO_ENTRY();
			// TODO
			/*auto d3d12ViewDesc = GetD3D12RenderTargetViewDesc(desc);
	
			m_descriptor = D3D12RenderAPI::Get()->AllocateCPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			D3D12RenderAPI::Get()->GetD3DDevice()->CreateUnorderedAccessView(static_cast<D3D12Texture*>(texture)->GetD3D12Resource(),
																			 &d3d12ViewDesc, m_descriptor.GetCPUHandle());*/
		}
		break;
	case TextureViewType::RTV:
		{
			auto d3d12ViewDesc = GetD3D12RenderTargetViewDesc(m_viewDesc);

			D3D12RenderDevice::Get().GetD3D12Device()->CreateRenderTargetView(
				static_cast<D3D12Texture*>(m_owningTexture.Raw())->GetD3D12Resource(),
				&d3d12ViewDesc, m_descriptor.GetCPUHandle());
		}
		break;
	case TextureViewType::DSV:
		{
			auto d3d12ViewDesc = GetD3D12DepthStencilViewDesc(m_viewDesc);

			D3D12RenderDevice::Get().GetD3D12Device()->CreateDepthStencilView(
				static_cast<D3D12Texture*>(m_owningTexture.Raw())->GetD3D12Resource(),
				&d3d12ViewDesc, m_descriptor.GetCPUHandle());
		}
		break;
	default: ;
	}
}

void D3D12TextureView::BuildView()
{
}
}
