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
struct VertexData
{
	glm::float3 position;
	glm::float3 normal;
	glm::float3 tangent;
	glm::float3 bitangent;
	glm::float2 texCoords;
	glm::float4 vertexColor;
};

enum class TextureType
{
	Albedo,
	Normals,
	Metallic,
	Roughness
};

struct PrimitiveData
{
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;

	std::unordered_map<TextureType, Ref<Render::Texture>> textures;

	Bounds3f bounds;
};

struct MeshData
{
	std::vector<PrimitiveData> primitives;
	Bounds3f bounds;
};

void InitMeshLoading();
MeshData LoadMeshFromFile(const std::wstring& filePath);

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

	MeshNode GetRootNode() const;

	int32_t GetNodeChildrenCount(MeshNode node) const;
	MeshNode GetNodeChild(MeshNode node, int32_t index) const;
	MeshNode GetNodeParent(MeshNode node) const;
	bool DoesNodeContainVertices(MeshNode node) const;
	int64_t GetNodeVertexCount(MeshNode node) const;

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
	Assimp::Importer m_importer;

	struct NodeInfo
	{
		std::variant<aiNode*, aiMesh*> assimpNode;
		MeshNode parent;
	};
	std::vector<NodeInfo> m_nodes;
};
}
