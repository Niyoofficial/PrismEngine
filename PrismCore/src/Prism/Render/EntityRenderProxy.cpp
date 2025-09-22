#include "EntityRenderProxy.h"

namespace Prism::Render
{
EntityRenderProxy::EntityRenderProxy(SceneRenderPipeline* renderPipeline)
	: m_renderPipeline(renderPipeline)
{
}

VertexFactory* EntityRenderProxy::GetVertexFactory() const
{
	PE_ASSERT(m_renderPipeline.IsValid());
	return m_renderPipeline->GetDefaultVertexFactory();
}
}
