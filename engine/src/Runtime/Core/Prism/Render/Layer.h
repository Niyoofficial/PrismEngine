#pragma once
#include "Prism/Utilities/Duration.h"

namespace Prism::Render
{
class Layer : public RefCounted
{
public:
	virtual void Attach();
	virtual void Detach();
	virtual void UpdateImGui(Duration delta);
	virtual void Update(Duration delta);
};
}
