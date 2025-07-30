#include "pcpch.h"
#include "PrimitiveBatch.h"

#include "Prism/Render/Primitive.h"

namespace Prism::Render
{
PrimitiveBatch::PrimitiveBatch(const std::wstring& primitiveBatchName)
	: m_primitiveBatchName(primitiveBatchName)
{
}

void PrimitiveBatch::AddPrimitive(int64_t vertexSize, IndexBufferFormat indexFormat,
								  void* vertexBuffer, int64_t vertexCount, void* indexBuffer, int64_t indexCount,
								  std::wstring primitiveBufferParamName, int64_t primitiveBufferSize,
								  const Material& material)
{
	Ref primitive = new Primitive(m_primitiveBatchName);
	primitive->SetVertices(vertexSize, vertexBuffer, vertexCount, indexFormat, indexBuffer, indexCount);
	primitive->SetPrimitiveUniformBuffer(primitiveBufferParamName, primitiveBufferSize);
	m_primitives.emplace_back(primitive, material);
}

void PrimitiveBatch::Draw(RenderContext* renderContext, void* cbufferData, int64_t dataSize)
{
	PE_ASSERT(renderContext);

	for (auto& [primitive, material]: m_primitives)
		primitive->DrawPrimitive(renderContext, cbufferData, dataSize, material);
}
}
