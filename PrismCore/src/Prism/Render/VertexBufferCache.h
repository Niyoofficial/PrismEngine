#pragma once
#include "Prism/Render/Buffer.h"
#include "Prism/Utilities/MeshLoading.h"

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
	MeshBuffers GetOrCreateMeshBuffers(const VertexAttributeList& attributeList, MeshLoading::MeshAsset* mesh);
	NodeInfo GetNodeIndexInfo(MeshLoading::MeshAsset* mesh, MeshLoading::MeshNode node);

private:
	void CreateVertexBuffer(const VertexAttributeList& attributeList, MeshLoading::MeshAsset* mesh);
	void CreateIndexBuffer(MeshLoading::MeshAsset* mesh);

private:
	// TODO: Figure out a way to not store a hard ref to meshes
	std::unordered_map<VertexAttributeList, std::unordered_map<Ref<MeshLoading::MeshAsset>, Ref<Buffer>>> m_vbCache;

	using NodesIndexInfo = std::unordered_map<MeshLoading::MeshNode, NodeInfo>;

	std::unordered_map<Ref<MeshLoading::MeshAsset>, std::pair<Ref<Buffer>, NodesIndexInfo>> m_ibCache;
};
}
