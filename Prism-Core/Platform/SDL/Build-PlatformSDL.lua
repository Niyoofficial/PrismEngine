project "Platform-SDL"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	files { "Src/**" }

	includeDirs["SDL"] = "%{prj.location}/Vendor/SDL/include"
	libDirs["SDL"] = "%{prj.location}/Vendor/SDL/Bin/" .. outputFolderName

	includedirs
	{
		"Src",

		includeDirs["Prism-Core"],

		includeDirs["glm"],
		includeDirs["SDL"],
		includeDirs["spdlog"]
	}

	libdirs
	{
		libDirs["SDL"],
		libDirs["spdlog"]
	}

	links
	{
		"SDL3.lib",
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
