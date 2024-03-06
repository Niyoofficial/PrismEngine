#pragma once

namespace Prism::Render
{
enum class ResourceType
{
	Buffer,
	Texture
};

class RenderResource
{
public:
	virtual ResourceType GetResourceType() const = 0;
};
}
