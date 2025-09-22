#pragma once

namespace Prism::Render
{
class SceneRenderPipeline
{
public:
	virtual ~SceneRenderPipeline() = default;

	virtual class VertexFactory* GetDefaultVertexFactory() const = 0;
};
}
