#pragma once


namespace Prism
{
namespace MeshLoading
{
class MeshAsset;
}

/**
 * VertexFactory is a class that describes what vertex attributes should be included in the vertex buffer,
 * that way we can keep a single instance of a mesh in memory that contains all possible attributes,
 * but before the vertex buffer is created it will go through this class to separate only the ones that
 * are actually needed in the shader
 */
class VertexFactory : public RefCounted
{
private:
	VertexFactory() = default;

	virtual std::vector<uint8_t> GatherVertexAttributes(MeshLoading::MeshAsset* meshAsset, int32_t primitiveIndex) const = 0;
};
}
