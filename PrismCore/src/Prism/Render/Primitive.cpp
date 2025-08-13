#include "pcpch.h"
#include "Primitive.h"

#include "Prism/Render/Buffer.h"
#include "Prism/Render/Material.h"

namespace Prism::Render
{
Primitive::Primitive(const std::wstring& primitiveName)
	: m_primitiveName(primitiveName)
{
}

void Primitive::SetVertices(int64_t vertexSize, void* vertexBuffer, int64_t vertexCount,
							IndexBufferFormat indexFormat, void* indexBuffer, int64_t indexCount)
{
	PE_ASSERT(vertexSize > 0);
	PE_ASSERT(vertexBuffer);
	PE_ASSERT(vertexCount > 0);
	PE_ASSERT(indexBuffer);
	PE_ASSERT(indexCount > 0);

	m_vertexSize = vertexSize;
	m_vertexBuffer = Buffer::Create(
		{
			.bufferName = m_primitiveName + L"_VertexBuffer",
			.size = vertexCount * vertexSize,
			.bindFlags = BindFlags::VertexBuffer
		},
		{
			.data = vertexBuffer,
			.sizeInBytes = vertexCount * vertexSize
		});

	m_indexFormat = indexFormat;
	int64_t indexSizeBytes = indexFormat == IndexBufferFormat::Uint16 ? sizeof(uint16_t) : sizeof(uint32_t);
	m_indexBuffer = Buffer::Create(
		{
			.bufferName = m_primitiveName + L"_IndexBuffer",
			.size = indexCount * indexSizeBytes,
			.bindFlags = BindFlags::IndexBuffer
		},
		{
			.data = indexBuffer,
			.sizeInBytes = indexCount * indexSizeBytes
		});
}

void Primitive::SetBounds(Bounds3F bounds)
{
	m_bounds = bounds;
}

void Primitive::SetPrimitiveUniformBuffer(std::wstring bufferParamName, int64_t bufferSize)
{
	m_primitiveUniformBuffer = Buffer::Create(
		{
			.bufferName = m_primitiveName + L"_UniformBuffer",
			.size = bufferSize,
			.bindFlags = BindFlags::UniformBuffer,
			.usage = ResourceUsage::Dynamic,
			.cpuAccess = CPUAccess::Write
		});
	m_primitiveUniformBufferView = m_primitiveUniformBuffer->CreateDefaultCBVView();
	m_primitiveParamName = bufferParamName;
}

void Primitive::DrawPrimitive(RenderContext* renderContext, void* uniformBufferData, int64_t dataSize, Material material, bool bBindMaterial)
{
	PE_ASSERT(m_vertexBuffer);
	PE_ASSERT(m_vertexSize > 0);
	PE_ASSERT(m_indexBuffer);
	PE_ASSERT(m_indexFormat != IndexBufferFormat::Unknown);

	PE_ASSERT(!m_primitiveParamName.empty());
	PE_ASSERT(m_primitiveUniformBuffer);
	PE_ASSERT(m_primitiveUniformBufferView);

	PE_ASSERT(renderContext);
	PE_ASSERT(dataSize <= m_primitiveUniformBuffer->GetBufferDesc().size, "You cannot pass more data than it was set in SetPrimitiveUniformBuffer");

	if (uniformBufferData && dataSize > 0)
	{
		void* data = m_primitiveUniformBuffer->Map(CPUAccess::Write);
		memcpy_s(data, m_primitiveUniformBuffer->GetBufferDesc().size, uniformBufferData, dataSize);
		m_primitiveUniformBuffer->Unmap();
		renderContext->SetBuffer(m_primitiveUniformBufferView, m_primitiveParamName);
	}

	renderContext->SetVertexBuffer(m_vertexBuffer, m_vertexSize);
	renderContext->SetIndexBuffer(m_indexBuffer, m_indexFormat);

	if (bBindMaterial)
		material.BindMaterial(renderContext);

	renderContext->DrawIndexed({
		.numIndices = m_indexBuffer->GetBufferDesc().size / (int64_t)sizeof(int32_t),
		.numInstances = 1,
		.startIndexLocation = 0,
		.baseVertexLocation = 0
	});
}
}
