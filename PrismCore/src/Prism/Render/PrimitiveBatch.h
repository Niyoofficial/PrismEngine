#pragma once
#include "Prism/Render/BufferView.h"
#include "Prism/Render/Primitive.h"

namespace Prism::Render
{
enum class IndexBufferFormat;
class RenderContext;
class Primitive;

// A collection of primitives
class PrimitiveBatch : public RefCounted
{
public:
	explicit PrimitiveBatch(const std::wstring& primitiveBatchName,
							const std::wstring& primitiveCBufferParamName,
							int64_t cbufferSize);

	void AddPrimitive(int64_t vertexSize, IndexBufferFormat indexFormat,
					  void* vertexBuffer, int64_t vertexCount, void* indexBuffer, int64_t indexCount,
					  Primitive::TextureParameter albedoTexture = {}, Primitive::TextureParameter normalsTexture = {},
					  Primitive::TextureParameter metallicTexture = {}, Primitive::TextureParameter roughnessTexture = {});
	const std::vector<Ref<Primitive>>& GetPrimitives() const { return m_primitives; }

	void Draw(RenderContext* renderContext, void* cbufferData = nullptr, int64_t dataSize = -1);

private:
	std::wstring m_primitiveBatchName;

	Ref<Buffer> m_primitiveCBuffer;
	Ref<BufferView> m_primitiveCBufferView;
	std::wstring m_primitiveCBufferParamName;

	std::vector<Ref<Primitive>> m_primitives;
};
}
