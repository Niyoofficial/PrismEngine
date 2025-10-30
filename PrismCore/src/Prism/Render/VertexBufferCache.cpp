#include "VertexBufferCache.h"

#include "Prism/Render/Buffer.h"

namespace Prism::Render
{
int64_t GetVertexSize(const VertexAttributeList& attributeList)
{
	int64_t size = 0;
	for (auto att : attributeList)
	{
		switch (att)
		{
		case VertexAttribute::Position:
			size += sizeof(glm::float3);
			break;
		case VertexAttribute::Normal:
			size += sizeof(glm::float3);
			break;
		case VertexAttribute::TexCoord:
			size += sizeof(glm::float2);
			break;
		case VertexAttribute::Tangent:
			size += sizeof(glm::float3);
			break;
		case VertexAttribute::Bitangent:
			size += sizeof(glm::float3);
			break;
		case VertexAttribute::Color:
			size += sizeof(glm::float4);
			break;
		}
	}
	return size;
}

VertexBufferCache::MeshBuffers VertexBufferCache::GetOrCreateMeshBuffers(const VertexAttributeList& attributeList, MeshLoading::MeshAsset* mesh)
{
	Buffer* vertexBuffer = nullptr;
	Buffer* indexBuffer = nullptr;
	if (m_vbCache.contains(attributeList))
	{
		if (m_vbCache.at(attributeList).contains(mesh))
		{
			vertexBuffer = m_vbCache.at(attributeList).at(mesh);
		}
		else
		{
			CreateVertexBuffer(attributeList, mesh);
			vertexBuffer = m_vbCache.at(attributeList).at(mesh);
		}
	}
	else
	{
		CreateVertexBuffer(attributeList, mesh);
		vertexBuffer = m_vbCache.at(attributeList).at(mesh);
	}

	if (m_ibCache.contains(mesh))
	{
		indexBuffer = m_ibCache.at(mesh).first;
	}
	else
	{
		CreateIndexBuffer(mesh);
		indexBuffer = m_ibCache.at(mesh).first;
	}

	return {vertexBuffer, indexBuffer};
}

VertexBufferCache::NodeInfo VertexBufferCache::GetNodeIndexInfo(MeshLoading::MeshAsset* mesh, MeshLoading::MeshNode node)
{
	return m_ibCache.at(mesh).second.at(node);
}

void VertexBufferCache::CreateVertexBuffer(const VertexAttributeList& attributeList, MeshLoading::MeshAsset* mesh)
{
	std::vector<uint8_t> vertices;
	for (auto node : *mesh)
	{
		auto appendAttribute =
			[&vertices](auto att)
			{
				size_t startOffset = vertices.size();
				vertices.insert(vertices.end(), sizeof(att), 0);
				memcpy(vertices.data() + startOffset, &att, sizeof(att));
			};

		if (mesh->DoesNodeContainVertices(node))
		{
			for (int32_t i = 0; i < mesh->GetNodeVertexCount(node); ++i)
			{
				for (auto att : attributeList)
				{
					switch (att)
					{
					case VertexAttribute::Position:
						appendAttribute(mesh->GetPosition(node, i));
						break;
					case VertexAttribute::Normal:
						appendAttribute(mesh->GetNormal(node, i));
						break;
					case VertexAttribute::TexCoord:
						appendAttribute(mesh->GetTexCoord(node, i));
						break;
					case VertexAttribute::Tangent:
						appendAttribute(mesh->GetTangent(node, i));
						break;
					case VertexAttribute::Bitangent:
						appendAttribute(mesh->GetBitangent(node, i));
						break;
					case VertexAttribute::Color:
						appendAttribute(mesh->GetColor(node, i));
						break;
					}
				}
			}
		}
	}

	m_vbCache[attributeList][mesh] = Buffer::Create({
														.bufferName = L"VertexBuffer",
														.size = (int64_t)vertices.size(),
														.bindFlags = BindFlags::VertexBuffer,
													},
													{
														.data = vertices.data(),
														.sizeInBytes = (int64_t)vertices.size()
													});
}

void VertexBufferCache::CreateIndexBuffer(MeshLoading::MeshAsset* mesh)
{
	std::vector<uint32_t> indices;
	int32_t baseVertex = 0;
	for (auto node : (*mesh))
	{
		if (mesh->DoesNodeContainVertices(node))
		{
			m_ibCache[mesh].second[node] = {(int64_t)indices.size(), baseVertex};

			std::vector<uint32_t> nodeIndices = mesh->GetIndices(node);
			indices.insert(indices.end(), nodeIndices.begin(), nodeIndices.end());

			baseVertex += mesh->GetNodeVertexCount(node);
		}
	}

	m_ibCache[mesh].first = Buffer::Create({
											   .bufferName = L"IndexBuffer",
											   .size = (int64_t)(indices.size() * sizeof(uint32_t)),
											   .bindFlags = BindFlags::IndexBuffer,
										   },
										   {
											   .data = indices.data(),
											   .sizeInBytes = (int64_t)(indices.size() * sizeof(uint32_t))
										   });
}
}
