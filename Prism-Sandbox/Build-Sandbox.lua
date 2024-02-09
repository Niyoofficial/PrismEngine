project "Prism-Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	files { "Src/**" }

	includedirs
	{
		"Src",

		includeDirs["Prism-Core"],

		includeDirs["glm"],
		includeDirs["spdlog"]
	}

	libdirs
	{
		libDirs["spdlog"]
	}

	links
	{
		"Prism-Core",
		"spdlog.lib"
	}

	filter "system:windows"
		systemversion "latest"
		defines { "PE_PLATFORM_WINDOWS" }

		links
		{
			"Winmm.lib",
			"Version.lib",
			"Imm32.lib",
			"Cfgmgr32.lib"
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
