#pragma once

namespace Prism::Render
{
class Texture;
}

namespace Prism::MeshUtils
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

	Bounds3F bounds;
};

struct MeshData
{
	std::vector<PrimitiveData> primitives;
	Bounds3F bounds;
};

void InitMeshLoading();
MeshData LoadMeshFromFile(const std::wstring& filePath);
}
