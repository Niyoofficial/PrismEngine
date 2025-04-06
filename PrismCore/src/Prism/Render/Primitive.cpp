#include "pcpch.h"
#include "Primitive.h"

#include "Prism/Render/Buffer.h"

namespace Prism::Render
{
bool Primitive::TextureParameter::IsValid() const
{
	return textureView && !paramName.empty();
}

Primitive::Primitive(const std::wstring& primitiveName, int64_t vertexSize, IndexBufferFormat indexFormat,
					 void* vertexBuffer, int64_t vertexCount, void* indexBuffer, int64_t indexCount,
					 TextureParameter albedoTexture, TextureParameter normalsTexture,
					 TextureParameter metallicTexture, TextureParameter roughnessTexture,
					 Buffer* primitiveCBuffer, BufferView* primitiveCBufferView,
					 const std::wstring& primitiveCBufferParamName)
	: m_primitiveName(primitiveName),
	  m_vertexSize(vertexSize),
	  m_vertexCount(vertexCount), m_indexCount(indexCount),
	  m_indexFormat(indexFormat),
	  m_albedoTexture(albedoTexture), m_normalsTexture(normalsTexture),
	  m_metallicTexture(metallicTexture), m_roughnessTexture(roughnessTexture),
	  m_primitiveCBuffer(primitiveCBuffer), m_primitiveCBufferView(primitiveCBufferView),
	  m_primitiveCBufferParamName(primitiveCBufferParamName)
{
	PE_ASSERT(vertexSize > 0);
	PE_ASSERT(vertexBuffer);
	PE_ASSERT(vertexCount > 0);
	PE_ASSERT(indexBuffer);
	PE_ASSERT(indexCount > 0);

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

void Primitive::SetCBuffer(Buffer* primitiveCBuffer, BufferView* primitiveCBufferView,
						   const std::wstring& primitiveCBufferParamNam)
{
	m_primitiveCBuffer = primitiveCBuffer;
	m_primitiveCBufferView = primitiveCBufferView;
	m_primitiveCBufferParamName = primitiveCBufferParamNam;
}

void Primitive::SetAlbedoTexture(TextureParameter albedo)
{
	m_albedoTexture = albedo;
}

void Primitive::SetNormalsTexture(TextureParameter normals)
{
	m_normalsTexture = normals;
}

void Primitive::SetMetallicTexture(TextureParameter metallic)
{
	m_metallicTexture = metallic;
}

void Primitive::SetRoughnessTexture(TextureParameter roughness)
{
	m_roughnessTexture = roughness;
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

	if (m_albedoTexture.IsValid())
		renderContext->SetTexture(m_albedoTexture.textureView, m_albedoTexture.paramName);
	if (m_normalsTexture.IsValid())
		renderContext->SetTexture(m_normalsTexture.textureView, m_normalsTexture.paramName);
	if (m_metallicTexture.IsValid())
		renderContext->SetTexture(m_metallicTexture.textureView, m_metallicTexture.paramName);
	if (m_roughnessTexture.IsValid())
		renderContext->SetTexture(m_roughnessTexture.textureView, m_roughnessTexture.paramName);

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
