#include "PBRSceneRenderPipeline.h"

namespace Prism::Render
{
class PBRVertexFactory : public VertexFactory
{
public:
	virtual std::vector<VertexAttribute> GatherVertexAttributes() const override
	{
		
	}
};

PBRSceneRenderPipeline::PBRSceneRenderPipeline()
	: m_defaultVertexFactory(new PBRVertexFactory)
{
}

VertexFactory* PBRSceneRenderPipeline::GetDefaultVertexFactory() const
{
	return m_defaultVertexFactory;
}
}
