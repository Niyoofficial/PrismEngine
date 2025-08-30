#include "Scene.h"

namespace Prism
{
void Scene::AddPrimitiveBatch(Render::PrimitiveBatch* batch)
{
	m_primitiveBatches.push_back(batch);
}
}
