#pragma once
#include <variant>

#include "assimp/Importer.hpp"

struct aiMesh;
struct aiNode;

namespace Prism::Render
{
class Texture;
}

namespace Prism::MeshLoading
{
enum class TextureType
{
	Albedo,
	Normals,
	Metallic,
	Roughness
};

void InitMeshLoading();

using MeshNode = int32_t;

struct MeshNodeIterator
{
	MeshNodeIterator(MeshNode node);

	constexpr auto operator<=>(MeshNodeIterator const&) const = default;
	MeshNode operator*() const;
	void operator++();
	void operator--();

	MeshNode value = -1;
};

class MeshAsset : public RefCounted
{
public:
	explicit MeshAsset(const std::wstring& filePath);

	std::wstring GetLoadedFilepath() const;

	MeshNode GetRootNode() const;
	int32_t GetTotalNodeCount() const;

	int32_t GetNodeChildrenCount(MeshNode node) const;
	MeshNode GetNodeChild(MeshNode node, int32_t index) const;
	MeshNode GetNodeParent(MeshNode node) const;
	bool DoesNodeContainVertices(MeshNode node) const;
	int64_t GetNodeVertexCount(MeshNode node) const;
	std::wstring GetNodeName(MeshNode node) const;

	Render::Texture* GetNodeTexture(MeshNode node, TextureType type);

	Bounds3f GetBoundingBox(MeshNode node) const;

	glm::float3 GetPosition(MeshNode node, int32_t vertexIndex) const;
	glm::float3 GetNormal(MeshNode node, int32_t vertexIndex) const;
	glm::float2 GetTexCoord(MeshNode node, int32_t vertexIndex, int32_t texCoordIndex = 0) const;
	glm::float3 GetTangent(MeshNode node, int32_t vertexIndex) const;
	glm::float3 GetBitangent(MeshNode node, int32_t vertexIndex) const;
	glm::float4 GetColor(MeshNode node, int32_t vertexIndex, int32_t colorIndex = 0) const;

	std::vector<uint32_t> GetIndices(MeshNode node) const;
	int64_t GetIndexCount(MeshNode node) const;

	MeshNodeIterator begin() const;
	MeshNodeIterator end() const;

private:
	std::wstring m_filePath;

	Assimp::Importer m_importer;

	struct NodeInfo
	{
		std::variant<aiNode*, aiMesh*> assimpNode;
		MeshNode parent;
		std::unordered_map<TextureType, Ref<Render::Texture>> textures;
	};
	std::vector<NodeInfo> m_nodes;
};
}
