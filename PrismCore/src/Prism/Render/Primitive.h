#pragma once
#include "Prism/Render/RenderContext.h"

namespace Prism::Render
{
class Primitive : public RefCounted
{
public:
	struct TextureParameter
	{
		bool IsValid() const;

		Ref<TextureView> textureView;
		std::wstring paramName;
	};

public:
	Primitive(const std::wstring& primitiveName, int64_t vertexSize, IndexBufferFormat indexFormat,
			  void* vertexBuffer, int64_t vertexCount, void* indexBuffer, int64_t indexCount,
			  TextureParameter albedoTexture = {}, TextureParameter normalsTexture = {},
			  TextureParameter metallicTexture = {}, TextureParameter roughnessTexture = {},
			  Buffer* primitiveCBuffer = nullptr, BufferView* primitiveCBufferView = nullptr,
			  const std::wstring& primitiveCBufferParamNam = L"");

	void SetCBuffer(Buffer* primitiveCBuffer,
					BufferView* primitiveCBufferView,
					const std::wstring& primitiveCBufferParamNam);

	void SetAlbedoTexture(TextureParameter albedo);
	void SetNormalsTexture(TextureParameter normals);
	void SetMetallicTexture(TextureParameter metallic);
	void SetRoughnessTexture(TextureParameter roughness);

	void BindPrimitive(RenderContext* renderContext, void* cbufferData = nullptr, int64_t dataSize = -1);
	void DrawPrimitive(RenderContext* renderContext);

protected:
	std::wstring m_primitiveName;

	Ref<Buffer> m_vertexBuffer;
	int64_t m_vertexSize = -1;
	int64_t m_vertexCount = -1;

	Ref<Buffer> m_indexBuffer;
	int64_t m_indexCount = -1;
	IndexBufferFormat m_indexFormat = IndexBufferFormat::Uint32;

	TextureParameter m_albedoTexture;
	TextureParameter m_normalsTexture;
	TextureParameter m_metallicTexture;
	TextureParameter m_roughnessTexture;

	Ref<Buffer> m_primitiveCBuffer;
	Ref<BufferView> m_primitiveCBufferView;
	std::wstring m_primitiveCBufferParamName;
};
}
