project "RendererAPI-D3D12"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	files { "Src/**" }

	includedirs
	{
		"Src",

		includeDirs["glm"],
		includeDirs["spdlog"]
	}

	libdirs
	{
		libDirs["spdlog"]
	}

	links
	{
		"spdlog.lib"
	}

	filter "system:windows"
		systemversion "latest"
		defines { "PE_PLATFORM_WINDOWS" }

	filter "configurations:Debug"
		defines { "PE_BUILD_DEBUG" }
		runtime "Debug"
		optimize "Off"
		symbols "On"

	filter "configurations:Profile"
		defines { "PE_BUILD_PROFILE" }
		runtime "Release"
		optimize "On"
		symbols "On"

	filter "configurations:Release"
		defines { "PE_BUILD_RELEASE" }
		runtime "Release"
		optimize "Full"
		symbols "Off"
