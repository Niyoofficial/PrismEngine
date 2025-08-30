#pragma once
#include "Prism/Render/PrimitiveBatch.h"

namespace Prism
{
class Scene
{
public:
	void AddPrimitiveBatch(Render::PrimitiveBatch* batch);

	const std::vector<Render::PrimitiveBatch*>& GetPrimitiveBatches() const { return m_primitiveBatches; }

private:
	std::vector<Render::PrimitiveBatch*> m_primitiveBatches;
};
}
