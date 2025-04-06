#include "pcpch.h"
#include "PrimitiveBatch.h"

#include "Prism/Render/Primitive.h"

namespace Prism::Render
{
PrimitiveBatch::PrimitiveBatch(const std::wstring& primitiveBatchName,
							   const std::wstring& primitiveCBufferParamName,
							   int64_t cbufferSize)
	: m_primitiveBatchName(primitiveBatchName), m_primitiveCBufferParamName(primitiveCBufferParamName)
{
	m_primitiveCBuffer = Buffer::Create({
		.bufferName = primitiveBatchName + L"_CBuffer",
		.size = cbufferSize,
		.bindFlags = BindFlags::ConstantBuffer,
		.usage = ResourceUsage::Dynamic,
		.cpuAccess = CPUAccess::Write
	});
	m_primitiveCBufferView = m_primitiveCBuffer->CreateDefaultCBVView();
}

void PrimitiveBatch::AddPrimitive(int64_t vertexSize, IndexBufferFormat indexFormat,
								  void* vertexBuffer, int64_t vertexCount, void* indexBuffer, int64_t indexCount,
								  Primitive::TextureParameter albedoTexture, Primitive::TextureParameter normalsTexture,
								  Primitive::TextureParameter metallicTexture, Primitive::TextureParameter roughnessTexture)
{
	m_primitives.emplace_back(new Primitive(m_primitiveBatchName + std::to_wstring(m_primitives.size()),
											vertexSize, indexFormat, vertexBuffer, vertexCount, indexBuffer, indexCount,
											albedoTexture, normalsTexture, metallicTexture, roughnessTexture,
											m_primitiveCBuffer, m_primitiveCBufferView, m_primitiveCBufferParamName));
}

void PrimitiveBatch::Draw(RenderContext* renderContext, void* cbufferData, int64_t dataSize)
{
	PE_ASSERT(renderContext);

	for (Primitive* primitive : m_primitives)
	{
		primitive->BindPrimitive(renderContext, cbufferData, dataSize);
		primitive->DrawPrimitive(renderContext);
	}
}
}
