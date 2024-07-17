#pragma once
#include "Prism-Core/Render/RenderContext.h"

namespace Prism::Render
{
class Primitive : public RefCounted
{
public:
	Primitive(const std::wstring& primitiveName, int64_t vertexSize, IndexBufferFormat indexFormat,
			  void* vertexBuffer, int64_t vertexCount, void* indexBuffer, int64_t indexCount,
			  const std::wstring& primitiveCBufferParamName, int64_t primitiveCBufferSize);

	void BindPrimitive(RenderContext* renderContext, void* cbufferData = nullptr, int64_t dataSize = -1);
	void DrawPrimitive(RenderContext* renderContext);

protected:
	Ref<Buffer> m_vertexBuffer;
	Ref<Buffer> m_indexBuffer;
	Ref<Buffer> m_primitiveCBuffer;
	Ref<BufferView> m_primitiveCBufferView;

	std::wstring m_primitiveName;
	int64_t m_vertexSize = -1;
	IndexBufferFormat m_indexFormat = IndexBufferFormat::Uint32;
	int64_t m_vertexCount = -1;
	int64_t m_indexCount = -1;
	std::wstring m_primitiveCBufferParamName;
	int64_t m_cbufferSize = -1;
};
}
