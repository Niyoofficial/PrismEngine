#include "pcpch.h"
#include "Primitive.h"

#include "Prism-Core/Render/Buffer.h"

namespace Prism::Render
{
Primitive::Primitive(const std::wstring& primitiveName, int64_t vertexSize, IndexBufferFormat indexFormat,
					 void* vertexBuffer, int64_t vertexCount, void* indexBuffer, int64_t indexCount,
					 const std::wstring& primitiveCBufferParamName, int64_t primitiveCBufferSize)
	: m_primitiveName(primitiveName), m_vertexSize(vertexSize), m_indexFormat(indexFormat),
	  m_vertexCount(vertexCount), m_indexCount(indexCount),
	  m_primitiveCBufferParamName(primitiveCBufferParamName), m_cbufferSize(primitiveCBufferSize)
{
	PE_ASSERT(vertexSize > 0);
	PE_ASSERT(vertexBuffer);
	PE_ASSERT(vertexCount > 0);
	PE_ASSERT(indexBuffer);
	PE_ASSERT(indexCount > 0);
	PE_ASSERT(primitiveCBufferSize > 0);

	// Primitive CBuffer
	m_primitiveCBuffer = Buffer::Create({
											.bufferName = m_primitiveName + L"_CBuffer",
											.size = m_cbufferSize,
											.bindFlags = BindFlags::ConstantBuffer,
											.usage = ResourceUsage::Dynamic,
											.cpuAccess = CPUAccess::Write
										}, {});
	m_primitiveCBufferView = m_primitiveCBuffer->CreateDefaultCBVView();

	// Vertex Buffer
	m_vertexBuffer = Buffer::Create({
										.bufferName = m_primitiveName + L"_VertexBuffer",
										.size = m_vertexCount * m_vertexSize,
										.bindFlags = BindFlags::VertexBuffer
									},
									{
										.data = vertexBuffer,
										.sizeInBytes = m_vertexCount * m_vertexSize
									});

	// Index Buffer
	int64_t indexSizeBytes = m_indexFormat == IndexBufferFormat::Uint16 ? sizeof(uint16_t) : sizeof(uint32_t);
	m_indexBuffer = Buffer::Create({
									   .bufferName = m_primitiveName + L"_IndexBuffer",
									   .size = m_indexCount * indexSizeBytes,
									   .bindFlags = BindFlags::IndexBuffer
								   },
								   {
									   .data = indexBuffer,
									   .sizeInBytes = m_indexCount * indexSizeBytes
								   });
}

void Primitive::BindPrimitive(RenderContext* renderContext, void* cbufferData, int64_t dataSize)
{
	PE_ASSERT(renderContext);

	if (cbufferData && dataSize > 0)
	{
		void* data = m_primitiveCBuffer->Map(CPUAccess::Write);
		memcpy_s(data, m_primitiveCBuffer->GetBufferDesc().size, cbufferData, dataSize);
		m_primitiveCBuffer->Unmap();
		renderContext->SetBuffer(m_primitiveCBufferView, m_primitiveCBufferParamName);
	}

	renderContext->SetVertexBuffer(m_vertexBuffer, m_vertexSize);
	renderContext->SetIndexBuffer(m_indexBuffer, m_indexFormat);
}

void Primitive::DrawPrimitive(RenderContext* renderContext)
{
	renderContext->DrawIndexed({
		.numIndices = m_indexBuffer->GetBufferDesc().size / (int64_t)sizeof(int32_t),
		.numInstances = 1,
		.startIndexLocation = 0,
		.baseVertexLocation = 0
	});
}
}
