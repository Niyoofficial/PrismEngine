#include "pcpch.h"
#include "D3D12Texture.h"

#include "ResourceUploadBatch.h"
#include "RenderAPI/D3D12/D3D12RenderDevice.h"
#include "RenderAPI/D3D12/D3D12TypeConversions.h"

#include "WICTextureLoader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Prism-Core/Render/RenderCommandQueue.h"

namespace Prism::Render::D3D12
{
D3D12Texture::D3D12Texture(const TextureDesc& desc, RawData initData, BarrierLayout initLayout)
	: m_originalDesc(desc)
{
	PE_ASSERT(desc.usage != ResourceUsage::Staging);
	PE_ASSERT(desc.usage != ResourceUsage::Dynamic);

	auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC1 d3d12Desc = GetD3D12ResourceDesc(m_originalDesc);
	D3D12_CLEAR_VALUE d3d12ClearValue = m_originalDesc.optimizedClearValue.has_value()
											? GetD3D12ClearValue(m_originalDesc.optimizedClearValue.value())
											: D3D12_CLEAR_VALUE{};
	PE_ASSERT_HR(D3D12RenderDevice::Get().GetD3D12Device()->CreateCommittedResource3(
		&heapProps, D3D12_HEAP_FLAG_NONE,
		&d3d12Desc, GetD3D12BarrierLayout(initLayout),
		m_originalDesc.optimizedClearValue.has_value() ? &d3d12ClearValue : nullptr,
		nullptr, 0, nullptr,
		IID_PPV_ARGS(&m_resource)));

	PE_ASSERT_HR(m_resource->SetName(m_originalDesc.textureName.c_str()));

	if (initData.data && initData.sizeInBytes > 0)
	{
		auto context = D3D12RenderDevice::Get().AllocateContext();
		context->UpdateTexture(this, initData, 0);

		D3D12RenderDevice::Get().SubmitContext(context);
		D3D12RenderDevice::Get().GetRenderQueue()->Flush();
	}
}

D3D12Texture::D3D12Texture(std::wstring filepath, bool loadAsCubemap)
{
	DISABLE_DESTRUCTION_SCOPE_GUARD(this);

	std::wstring ext = filepath.substr(filepath.find_last_of('.', std::wstring::npos) + 1);
	if (ext == L"hdr")
	{
		//stbi_set_flip_vertically_on_load(true);

		int32_t width = -1;
		int32_t height = -1;
		int32_t channels = -1;

		float* loadedData = stbi_loadf(WStringToString(filepath).c_str(), &width, &height, &channels, 4);
		//stbi_set_flip_vertically_on_load(false);


		PE_ASSERT(loadedData);


		m_originalDesc = {
			.textureName = filepath,
			.width = width,
			.height = height,
			.depthOrArraySize = 1,
			.dimension = ResourceDimension::Tex2D,
			.format = TextureFormat::RGBA32_Float,
			.mipLevels = 0,
			.bindFlags = BindFlags::ShaderResource,
			.usage = ResourceUsage::Default
		};

		auto resDesc = GetD3D12ResourceDesc(m_originalDesc);
		auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		PE_ASSERT_HR(D3D12RenderDevice::Get().GetD3D12Device()->CreateCommittedResource3(
			&heapProps, D3D12_HEAP_FLAG_NONE,
			&resDesc, D3D12_BARRIER_LAYOUT_COMMON,
			nullptr,
			nullptr, 0, nullptr,
			IID_PPV_ARGS(&m_resource)));

		PE_ASSERT(m_resource);

		PE_ASSERT_HR(m_resource->SetName(filepath.c_str()));


		auto context = D3D12RenderDevice::Get().AllocateContext();
		context->UpdateTexture(this, {.data = loadedData, .sizeInBytes = (int64_t)(width * height * 4 * 4)}, 0);

		D3D12RenderDevice::Get().SubmitContext(context);
		D3D12RenderDevice::Get().GetRenderQueue()->Flush();
	}
	else
	{
		auto* d3d12Device = D3D12RenderDevice::Get().GetD3D12Device();
		DX::ResourceUploadBatch batch(d3d12Device);
		batch.Begin();
		PE_ASSERT_HR(DX::CreateWICTextureFromFile(d3d12Device, batch, filepath.c_str(), &m_resource, true));
		batch.End(D3D12RenderDevice::Get().GetD3D12CommandQueue()).wait_for(std::chrono::seconds(0));
	}

	PE_ASSERT(m_resource);

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
	if (m_resource)
	{
		return D3D12::GetTextureDesc(m_resource->GetDesc(), m_originalDesc.textureName,
									 m_originalDesc.usage, m_originalDesc.optimizedClearValue, m_originalDesc.IsCube());
	}

	return m_originalDesc;
}
}
