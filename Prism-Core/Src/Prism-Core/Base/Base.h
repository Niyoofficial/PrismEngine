#pragma once

#include "Prism-Core/Base/Logging.h"
#include "Prism-Core/Base/Assert.h"
#include "Prism-Core/Utilities/StringUtils.h"
#include "Prism-Core/Utilities/MathUtils.h"
#include "Prism-Core/Utilities/Flags.h"

#include "glm/gtx/compatibility.hpp"

#ifndef PE_PLATFORM_WINDOWS
#error Prism only supports Windows for now
#endif

DECLARE_LOG_CATEGORY(PECore, "Prism-Core");
#define PE_CORE_LOG(verbosity, ...) PE_LOG(PECore, verbosity, __VA_ARGS__)

namespace Prism::Core
{
void InitCore();
void ShutdownCore();
}
