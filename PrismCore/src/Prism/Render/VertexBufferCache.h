#pragma once
#include "Prism/AssetManagement/MeshAsset.h"
#include "Prism/Render/Buffer.h"

namespace Prism::Render
{
enum class VertexAttribute
{
	Position,
	Normal,
	TexCoord,
	Tangent,
	Bitangent,
	Color
};

using VertexAttributeList = std::vector<VertexAttribute>;

int64_t GetVertexSize(const VertexAttributeList& attributeList);

class VertexBufferCache
{
public:
	struct MeshBuffers
	{
		Buffer* vertexBuffer;
		Buffer* indexBuffer;
	};

	struct NodeInfo
	{
		int64_t startIndex = -1;
		int64_t baseVertex = -1;
	};

public:
	MeshBuffers GetOrCreateMeshBuffers(const VertexAttributeList& attributeList, MeshAsset* mesh);
	NodeInfo GetNodeIndexInfo(MeshAsset* mesh, MeshNode node);

private:
	void CreateVertexBuffer(const VertexAttributeList& attributeList, MeshAsset* mesh);
	void CreateIndexBuffer(MeshAsset* mesh);

private:
	// TODO: Figure out a way to not store a hard ref to meshes
	std::unordered_map<VertexAttributeList, std::unordered_map<Ref<MeshAsset>, Ref<Buffer>>> m_vbCache;

	using NodesIndexInfo = std::unordered_map<MeshNode, NodeInfo>;

	std::unordered_map<Ref<MeshAsset>, std::pair<Ref<Buffer>, NodesIndexInfo>> m_ibCache;
};
}
