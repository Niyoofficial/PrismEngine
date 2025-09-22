#pragma once
#include "Prism/Render/SceneRenderPipeline.h"

namespace Prism::Render
{
class VertexFactory;

class EntityRenderProxy : public RefCounted
{
public:
	explicit EntityRenderProxy(SceneRenderPipeline* renderPipeline);

	VertexFactory* GetVertexFactory() const;

protected:
	WeakRef<SceneRenderPipeline> m_renderPipeline;
};
}
