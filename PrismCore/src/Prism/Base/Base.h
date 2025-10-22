#pragma once

#include "Prism/Base/Logging.h"
#include "Prism/Base/Assert.h"
#include "Prism/Base/Ref.h"
#include "Prism/Utilities/StringUtils.h"
#include "Prism/Utilities/MathUtils.h"
#include "Prism/Utilities/PreprocessorUtils.h"
#include "Prism/Utilities/Flags.h"
#include "Prism/Base/CommonTypes.h"

#include "Prism/UI/PrismImGui.h"

#include "glm/gtx/compatibility.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/matrix_decompose.hpp"

#include "yaml-cpp/yaml.h"

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

namespace glm
{
typedef vec<2, uint, highp> uint2;
typedef vec<3, uint, highp> uint3;
typedef vec<4, uint, highp> uint4;
}

namespace Prism::Core
{
void InitCore();
void ShutdownCore();
}

namespace std
{
	namespace fs = std::filesystem;
}
