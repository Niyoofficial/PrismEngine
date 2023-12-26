project "Prism-Core"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	pchheader "pcpch.h"
	pchsource "Src/pcpch.cpp"

	files
	{
		"Src/Prism-Core/**",
		"Src/pcpch.h",
		"Src/pcpch.cpp"
	}

	includeDirs["Platform-SDL"] = "%{prj.location}/Platform/SDL/Src"
	libDirs["Platform-SDL"] = "%{prj.location}/Platform/SDL/Bin/" .. outputFolderName

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

		files
		{
			"Src/Platform/SDL/**"
		}

		includedirs
		{
			includeDirs["Platform-SDL"]
		}

		libdirs
		{
			libDirs["Platform-SDL"]
		}

		links
		{
			"Platform-Test",
			"Platform-SDL"
		}

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
	filter {}
