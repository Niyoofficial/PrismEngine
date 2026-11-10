#pragma once
#include "Prism/Render/Material.h"
#include "Prism/Render/RenderContext.h"

namespace Prism::Render
{
class Primitive : public RefCounted
{
public:
	explicit Primitive(const std::wstring& primitiveName);

	void SetVertices(int64_t vertexSize, void* vertexBuffer, int64_t vertexCount, IndexBufferFormat indexFormat, void* indexBuffer, int64_t indexCount);
	void SetBounds(Bounds3f bounds);
	void SetPrimitiveUniformBuffer(std::wstring bufferParamName, int64_t bufferSize);

	Bounds3f GetBounds() const { return m_bounds; }

	void DrawPrimitive(RenderContext* renderContext, void* uniformBufferData, int64_t dataSize, Material material, bool bBindMaterial = true);

protected:
	std::wstring m_primitiveName;

	Ref<Buffer> m_vertexBuffer;
	int64_t m_vertexSize = -1;

	IndexBufferFormat m_indexFormat = IndexBufferFormat::Unknown;
	Ref<Buffer> m_indexBuffer;
	Bounds3f m_bounds;

	Ref<Buffer> m_primitiveUniformBuffer;
	Ref<BufferView> m_primitiveUniformBufferView;
	std::wstring m_primitiveParamName;
};
}
