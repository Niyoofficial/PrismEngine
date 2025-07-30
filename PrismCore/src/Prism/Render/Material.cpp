#include "Material.h"

#include "Prism/Render/Buffer.h"
#include "Prism/Render/RenderConstants.h"
#include "Prism/Render/RenderContext.h"
#include "Prism/Render/Texture.h"
#include "Prism/Utilities/MemoryUtils.h"

namespace Prism::Render
{
void Material::SetVertexShader(ShaderDesc shaderDesc)
{
	m_vertexShaderDesc = shaderDesc;
}

void Material::SetPixelShader(ShaderDesc shaderDesc)
{
	m_pixelShaderDesc = shaderDesc;
}

void Material::SetBuffer(const std::wstring& paramName, Ref<BufferView> buffer)
{
	PE_ASSERT(buffer);

	m_params[paramName] = buffer.Raw();
}

void Material::SetTexture(const std::wstring& paramName, Ref<TextureView> texture)
{
	PE_ASSERT(texture);
	m_params[paramName] = texture.Raw();
}

void Material::SetDynamicData(const std::wstring& paramName, void* data, int64_t size)
{
	PE_ASSERT(data);
	PE_ASSERT(size > 0 && size == Align(size, Constants::CBUFFER_ALIGNMENT));

	if (!m_params.contains(paramName))
	{
		auto buffer = Buffer::Create(
			{
				.bufferName = L"",
				.size = size,
				.bindFlags = BindFlags::UniformBuffer,
				.usage = ResourceUsage::Dynamic,
				.cpuAccess = CPUAccess::Write
			},
			{
				.data = data,
				.sizeInBytes = size
			});

		m_params[paramName] = DynamicDataInfo{
			.cbuffer = buffer,
			.view = buffer->CreateDefaultCBVView()
		};
	}
	else
	{
		PE_ASSERT(std::holds_alternative<DynamicDataInfo>(m_params[paramName]));

		auto& dataInfo = std::get<DynamicDataInfo>(m_params[paramName]);
		PE_ASSERT(dataInfo.cbuffer);
		void* address = dataInfo.cbuffer->Map(CPUAccess::Write);
		memcpy_s(address, dataInfo.cbuffer->GetBufferDesc().size, data, size);
		dataInfo.cbuffer->Unmap();
	}
}

void Material::RemoveParam(const std::wstring& paramName)
{
	m_params.erase(paramName);
}

void Material::SetBlendState(const struct BlendStateDesc& blendState)
{
	m_blendState = blendState;
}

void Material::SetRasterizerState(const struct RasterizerStateDesc& rastserizerState)
{
	m_rastserizerState = rastserizerState;
}

void Material::SetDepthStencilState(const struct DepthStencilStateDesc& depthStencilState)
{
	m_depthStencilState = depthStencilState;
}

void Material::SetTopologyType(TopologyType topologyType)
{
	m_topologyType = topologyType;
}

void Material::BindMaterial(RenderContext* renderContext)
{
	PE_ASSERT(renderContext);

	renderContext->SetPSO(GraphicsPipelineStateDesc{
		.vs = m_vertexShaderDesc,
		.ps = m_pixelShaderDesc,
		.blendState = m_blendState,
		.rasterizerState = m_rastserizerState,
		.depthStencilState = m_depthStencilState,
		.primitiveTopologyType = m_topologyType
	});

	for (auto [name, param] : m_params)
	{
		if (std::holds_alternative<Ref<RenderResourceView>>(param))
		{
			auto* resourceView = std::get<Ref<RenderResourceView>>(param).Raw();
			PE_ASSERT(resourceView);
			if (resourceView->GetResourceType() == ResourceType::Texture)
			{
				auto* textureView = resourceView->GetSubType<TextureView>();
				renderContext->SetTexture(textureView, name);
			}
			else if (resourceView->GetResourceType() == ResourceType::Buffer)
			{
				auto* bufferView = resourceView->GetSubType<BufferView>();
				renderContext->SetBuffer(bufferView, name);
			}
		}
		else if (std::holds_alternative<DynamicDataInfo>(param))
		{
			auto& dataInfo = std::get<DynamicDataInfo>(param);
			PE_ASSERT(dataInfo.view);
			renderContext->SetBuffer(dataInfo.view, name);
		}
		else
		{
			PE_ASSERT_NO_ENTRY();
		}
	}
}
}
