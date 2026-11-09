#pragma once
#include <variant>

#include "Prism/Render/RenderTypes.h"
#include "Prism/Render/Shader.h"

namespace Prism::Render
{
class Buffer;
class RenderResourceView;
class TextureView;
class BufferView;

class Material
{
public:
	Material() = default;

	void SetVertexShader(ShaderDesc shaderDesc);
	void SetPixelShader(ShaderDesc shaderDesc);

	void SetBuffer(const std::wstring& paramName, Ref<BufferView> buffer);
	void SetTexture(const std::wstring& paramName, Ref<TextureView> texture);
	void SetDynamicData(const std::wstring& paramName, void* data, int64_t size);

	void RemoveParam(const std::wstring& paramName);

	void SetBlendState(const BlendStateDesc& blendState);
	void SetRasterizerState(const RasterizerStateDesc& rastserizerState);
	void SetDepthStencilState(const DepthStencilStateDesc& depthStencilState);
	void SetTopologyType(TopologyType topologyType);

	void BindMaterial(class RenderContext* renderContext);

private:
	ShaderDesc m_vertexShaderDesc;
	ShaderDesc m_pixelShaderDesc;

	struct DynamicDataInfo
	{
		Ref<Buffer> cbuffer;
		Ref<BufferView> view;
	};
	using ParamData = std::variant<Ref<RenderResourceView>, DynamicDataInfo>;

	std::unordered_map<std::wstring, ParamData> m_params;

	BlendStateDesc m_blendState;
	RasterizerStateDesc m_rastserizerState;
	DepthStencilStateDesc m_depthStencilState;
	TopologyType m_topologyType = TopologyType::TriangleList;
};
}
