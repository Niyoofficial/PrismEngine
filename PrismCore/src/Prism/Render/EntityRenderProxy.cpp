#include "EntityRenderProxy.h"

namespace Prism::Render
{
EntityRenderProxy::EntityRenderProxy(const RenderProxyInitInfo& initInfo)
	: m_worldTransform(initInfo.wordTransform),
	  m_bounds(initInfo.bounds),
	  m_meshAsset(initInfo.meshAsset),
	  m_meshNode(initInfo.meshNode)
{
}
}
