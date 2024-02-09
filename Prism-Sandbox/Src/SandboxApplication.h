#pragma once
#include <memory>

#include "Prism-Core/Base/Application.h"

namespace Prism::Core
{
class Window;
}

class SandboxApplication final : public Prism::Core::Application
{
public:
	virtual void Init() override;

private:
	std::unique_ptr<class Prism::Core::Window> m_window;
};
