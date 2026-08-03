#include "VertexBufferCache.h"

#include "Prism/Render/Buffer.h"

namespace Prism::Render
{
int64_t GetVertexSize(const VertexAttributeList& attributeList)
{
	int64_t size = 0;
	for (auto att : attributeList)
	{
		size += GetVertexAttributeSize(att);
	}
	return size;
}

uint32_t GetVertexAttributeSize(const VertexAttribute& attribute)
{
	switch (attribute)
	{
	case VertexAttribute::Position:
		return sizeof(glm::float3);
	case VertexAttribute::Normal:
		return sizeof(glm::float3);
	case VertexAttribute::TexCoord:
		return sizeof(glm::float2);
	case VertexAttribute::Tangent:
		return sizeof(glm::float3);
	case VertexAttribute::Bitangent:
		return sizeof(glm::float3);
	case VertexAttribute::Color:
		return sizeof(glm::float4);
	default:
		PE_ASSERT_NO_ENTRY("Unknown vertex attribute");
		return 0;
	}
}

uint32_t GetVertexLocation(const VertexAttribute attribute)
{
	switch (attribute)
	{
	case VertexAttribute::Position:
		return 0;
	case VertexAttribute::Normal:
		return 1;
	case VertexAttribute::TexCoord:
		return 2;
	case VertexAttribute::Tangent:
		return 3;
	case VertexAttribute::Bitangent:
		return 4;
	case VertexAttribute::Color:
		return 5;
	default:
		PE_ASSERT_NO_ENTRY("Unknown vertex attribute");
		return 0;
	}
}

VertexBufferCache::MeshBuffers VertexBufferCache::GetOrCreateMeshBuffers(const VertexAttributeList& attributeList,
                                                                         MeshAsset* mesh)
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

VertexBufferCache::NodeInfo VertexBufferCache::GetNodeIndexInfo(MeshAsset* mesh, MeshNode node)
{
	return m_ibCache.at(mesh).second.at(node);
}

void VertexBufferCache::CreateVertexBuffer(const VertexAttributeList& attributeList, MeshAsset* mesh)
{
	std::vector<uint8_t> vertices;
	for (auto node : *mesh)
	{
		auto appendAttribute = [&vertices](auto att)
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

	m_vbCache[attributeList][mesh] = Buffer::Create(
	    {
	        .bufferName = L"VertexBuffer",
	        .size = (int64_t)vertices.size(),
	        .bindFlags = BindFlags::VertexBuffer,
	    },
	    {.data = vertices.data(), .sizeInBytes = (int64_t)vertices.size()});
}

void VertexBufferCache::CreateIndexBuffer(MeshAsset* mesh)
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

	m_ibCache[mesh].first = Buffer::Create(
	    {
	        .bufferName = L"IndexBuffer",
	        .size = (int64_t)(indices.size() * sizeof(uint32_t)),
	        .bindFlags = BindFlags::IndexBuffer,
	    },
	    {.data = indices.data(), .sizeInBytes = (int64_t)(indices.size() * sizeof(uint32_t))});
}
} // namespace Prism::Render
