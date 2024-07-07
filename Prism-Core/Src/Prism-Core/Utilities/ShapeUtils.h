#pragma once

namespace Prism::ShapeUtils
{
struct VertexData
{
	glm::float3 position;
	glm::float3 normal;
	glm::float3 tangent;
	glm::float3 bitangent;
	glm::float2 texCoord;
	glm::float4 vertexColor;
};

struct ShapeData
{
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;
};

void InitShapeLoading();
ShapeData LoadShapeFromFile(const std::wstring& filename);
}
