#pragma once
#include "Prism/Render/SceneRenderPipeline.h"
#include "Prism/Render/VertexFactory.h"

namespace Prism::Render
{
class PBRSceneRenderPipeline : public SceneRenderPipeline
{
public:
	PBRSceneRenderPipeline();

	virtual VertexFactory* GetDefaultVertexFactory() const override;

private:
	Ref<VertexFactory> m_defaultVertexFactory;
};
}
