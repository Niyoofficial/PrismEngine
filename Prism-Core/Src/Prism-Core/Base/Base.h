#pragma once

#include "Prism-Core/Base/Logging.h"
#include "Prism-Core/Base/Assert.h"
#include "Prism-Core/Base/Ref.h"
#include "Prism-Core/Utilities/StringUtils.h"
#include "Prism-Core/Utilities/MathUtils.h"
#include "Prism-Core/Utilities/PreprocessorUtils.h"
#include "Prism-Core/Utilities/Flags.h"

#include "glm/gtx/compatibility.hpp"

#define IMPLEMENT_APPLICATION(appClass)						\
	void CreateApplication(int32_t argc, char** argv)		\
	{														\
		Core::Application::Create<appClass>(argc, argv);	\
	}

#ifndef PE_PLATFORM_WINDOWS
#error Prism only supports Windows for now
#endif

DECLARE_LOG_CATEGORY(PECore, "Prism-Core");
#define PE_CORE_LOG(verbosity, ...) PE_LOG(PECore, verbosity, __VA_ARGS__)

template<typename Type, size_t Size>
struct std::hash<std::array<Type, Size>>
{
	size_t operator()(const std::array<Type, Size>& array) const noexcept
	{
		size_t hash = 0;
		for (const auto& entry : array)
			hash ^= std::hash<Type>()(entry);

		return hash;
	}
};

template<typename Type>
struct std::hash<std::vector<Type>>
{
	size_t operator()(const std::vector<Type>& array) const noexcept
	{
		size_t hash = 0;
		for (const auto& entry : array)
			hash ^= std::hash<Type>()(entry);

		return hash;
	}
};

namespace Prism::Core
{
void InitCore();
void ShutdownCore();
}
