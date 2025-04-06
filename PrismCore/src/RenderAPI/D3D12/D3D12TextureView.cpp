#include "pcpch.h"
#include "D3D12TextureView.h"

#include "RenderAPI/D3D12/D3D12RenderDevice.h"
#include "RenderAPI/D3D12/D3D12Texture.h"
#include "RenderAPI/D3D12/D3D12TypeConversions.h"

namespace Prism::Render::D3D12
{
D3D12TextureView::D3D12TextureView(TextureViewDesc desc, Texture* texture)
	: m_viewDesc(desc)
{
	PE_ASSERT(texture, "Invalid texture!");

	m_owningTexture = texture;

	if (m_viewDesc.format == TextureFormat::Unknown)
		m_viewDesc.format = texture->GetTextureDesc().format;

	if (m_viewDesc.type == TextureViewType::Unknown)
	{
		constexpr Flags<BindFlags> TEXTURE_VIEW_POSSIBLE_FLAGS =
			Flags(BindFlags::ShaderResource) |
			Flags(BindFlags::UnorderedAccess) |
			Flags(BindFlags::RenderTarget) |
			Flags(BindFlags::DepthStencil);

		Flags<BindFlags> flags = texture->GetTextureDesc().bindFlags & TEXTURE_VIEW_POSSIBLE_FLAGS;

		PE_ASSERT(glm::bitCount(flags.GetUnderlyingType()) == 1,
				  "Can't determine view type, to automatically determine view type the owning texture must have exactly one of the view-compatible bind flags");

		if (flags.HasAllFlags(BindFlags::ShaderResource))
			m_viewDesc.type = TextureViewType::SRV;
		else if (flags.HasAllFlags(BindFlags::UnorderedAccess))
			m_viewDesc.type = TextureViewType::UAV;
		else if (flags.HasAllFlags(BindFlags::RenderTarget))
			m_viewDesc.type = TextureViewType::RTV;
		else if (flags.HasAllFlags(BindFlags::DepthStencil))
			m_viewDesc.type = TextureViewType::DSV;
	}

	m_descriptor = D3D12RenderDevice::Get().AllocateDescriptors(GetD3D12DescriptorHeapType(m_viewDesc.type));

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
		auto d3d12ViewDesc = GetD3D12UnorderedAccessViewDesc(m_viewDesc);

		D3D12RenderDevice::Get().GetD3D12Device()->CreateUnorderedAccessView(
			static_cast<D3D12Texture*>(m_owningTexture.Raw())->GetD3D12Resource(),
			nullptr,
			&d3d12ViewDesc, m_descriptor.GetCPUHandle());
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

void D3D12TextureView::BuildDynamicDescriptor(CD3DX12_CPU_DESCRIPTOR_HANDLE d3d12DescriptorHandle)
{
	PE_ASSERT_NO_ENTRY();
}

D3D12_DESCRIPTOR_HEAP_TYPE D3D12TextureView::GetDescriptorHeapType() const
{
	return GetD3D12DescriptorHeapType(m_viewDesc.type);
}
}
