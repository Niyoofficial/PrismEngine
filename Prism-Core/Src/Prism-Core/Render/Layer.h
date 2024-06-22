#pragma once
#include "Prism-Core/Utilities/Duration.h"

namespace Prism::Render
{
class Layer : public RefCounted
{
public:
	virtual void Attach();
	virtual void Detach();
	virtual void Update(Duration delta);
};
}
