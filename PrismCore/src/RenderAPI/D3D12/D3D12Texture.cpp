#include "pcpch.h"
#include "D3D12Texture.h"

#include "ResourceUploadBatch.h"
#include "RenderAPI/D3D12/D3D12RenderDevice.h"
#include "RenderAPI/D3D12/D3D12TypeConversions.h"

#include "WICTextureLoader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Prism/Render/RenderCommandQueue.h"

#if PE_USE_PIX
#include "pix3.h"
#endif

namespace Prism::Render::D3D12
{
D3D12Texture::D3D12Texture(const TextureDesc& desc, BarrierLayout initLayout)
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
}

D3D12Texture::D3D12Texture(std::wstring filepath, bool loadAsCubemap, bool waitForLoadFinish)
{
	DISABLE_DESTRUCTION_SCOPE_GUARD(this);

	std::wstring ext = filepath.substr(filepath.find_last_of('.', std::wstring::npos) + 1);
	if (ext == L"hdr")
	{
		int32_t width = -1;
		int32_t height = -1;
		int32_t channels = -1;

		float* loadedData = stbi_loadf(WStringToString(filepath).c_str(), &width, &height, &channels, 4);

		PE_ASSERT(loadedData);

		m_originalDesc = {
			.textureName = filepath,
			.width = width,
			.height = height,
			.depthOrArraySize = 1,
			.mipLevels = (int32_t)std::log2f(max(width, height)) + 1,
			.dimension = ResourceDimension::Tex2D,
			.format = TextureFormat::RGBA32_Float,
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

		auto context = D3D12RenderDevice::Get().AllocateContext(std::format(L"LoadHDRImage_{}", filepath));
		context->UpdateTexture(this, {.data = loadedData, .sizeInBytes = (int64_t)(width * height * 4 * 4)}, 0);

		GenerateMipMaps(context);

		D3D12RenderDevice::Get().SubmitContext(context);
		D3D12RenderDevice::Get().GetRenderCommandQueue()->Flush();

		stbi_image_free(loadedData);

		PE_ASSERT(m_resource);

		auto loadedResDesc = m_resource->GetDesc();
		PE_ASSERT(!loadAsCubemap || resDesc.DepthOrArraySize == 6, "Cubemaps must have an array size of 6");

		m_originalDesc = D3D12::GetTextureDesc(loadedResDesc, filepath, ResourceUsage::Default, {}, loadAsCubemap);
	}
	else
	{
		m_originalDesc = {
			.textureName = filepath,
			.format = TextureFormat::RGBA32_Float,
			.bindFlags = BindFlags::ShaderResource,
			.usage = ResourceUsage::Default
		};

		ID3D12Device10* d3d12Device = D3D12RenderDevice::Get().GetD3D12Device();

		auto func =
			[loadAsCubemap, d3d12Device, filepath, this]()
			{
#if PE_USE_PIX
				// This will show up in the PIX event viewer incorrectly because its called asynchronously,
				// it'd work correctly if the event was called on the cmd list, but we cannot access is from ResourceUploadBatch
				// and I want to keep this function async since CreateWICTextureFromFileEx can take a while
				PIXBeginEvent(D3D12RenderDevice::Get().GetD3D12CommandQueue(), PIX_COLOR(0, 0, 0), std::format(L"LoadWICImage_{}", filepath).c_str());
#endif

				DX::ResourceUploadBatch batch(d3d12Device);
				batch.Begin();
				PE_ASSERT_HR(DX::CreateWICTextureFromFileEx(d3d12Device, batch, filepath.c_str(), 0,
					D3D12_RESOURCE_FLAG_NONE, DX::WIC_LOADER_FORCE_RGBA32 | DX::WIC_LOADER_MIP_AUTOGEN, &m_resource));
				batch.End(D3D12RenderDevice::Get().GetD3D12CommandQueue()).wait_for(std::chrono::seconds(0));

#if PE_USE_PIX
				PIXEndEvent(D3D12RenderDevice::Get().GetD3D12CommandQueue());
#endif

				PE_ASSERT(m_resource);

				auto resDesc = m_resource->GetDesc();
				PE_ASSERT(!loadAsCubemap || resDesc.DepthOrArraySize == 6, "Cubemaps must have an array size of 6");

				m_originalDesc = D3D12::GetTextureDesc(resDesc, filepath, ResourceUsage::Default, {}, loadAsCubemap);
			};

		if (waitForLoadFinish)
			func();
		else
			m_loadFuture = std::async(std::launch::async, func);
	}
}

D3D12Texture::D3D12Texture(ID3D12Resource* resource, const std::wstring& name, ResourceUsage usage,
						   ClearValue optimizedClearValue, bool isCubeTexture)
	: m_originalDesc(D3D12::GetTextureDesc(resource->GetDesc(), name, usage, optimizedClearValue, isCubeTexture)),
	  m_resource(resource)
{
	// Release here because assigning to a ComPtr will add another ref
	resource->Release();
	PE_ASSERT_HR(m_resource->SetName(m_originalDesc.textureName.c_str()));
}

void D3D12Texture::WaitForLoadFinish()
{
	if (m_loadFuture.valid())
	{
		m_loadFuture.wait_for(std::chrono::seconds(0));
		m_loadFuture = {};
	}
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
