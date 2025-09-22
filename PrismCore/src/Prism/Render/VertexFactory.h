#pragma once


namespace Prism::Render
{

enum class AttributeVariableType
{
	Int,
	Uint,
	Float,
};

struct VertexAttribute
{
	int32_t componentCount = 1;
	int32_t bytesPerComponent = 32;
	AttributeVariableType type = AttributeVariableType::Float;
};

/**
 * VertexFactory is a class that describes what vertex attributes should be included in the vertex buffer,
 * that way we can keep a single instance of a mesh in memory that contains all possible attributes,
 * but before the vertex buffer is created it will go through this class to separate only the ones that
 * are actually needed in the shader
 */
class VertexFactory : public RefCounted
{
public:
	VertexFactory() = default;

	virtual std::vector<VertexAttribute> GatherVertexAttributes() const = 0;
};
}
