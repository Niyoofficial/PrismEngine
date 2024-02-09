project "Prism-Core"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	pchheader "pcpch.h"
	pchsource "Src/pcpch.cpp"

	includeDirs["SDL"] = "%{prj.location}/Vendor/SDL/include"
	libDirs["SDL"] = "%{prj.location}/Vendor/SDL/Bin/" .. outputFolderName
	includeDirs["xxHash"] = "%{prj.location}/Vendor/xxHash"
	libDirs["xxHash"] = "%{prj.location}/Vendor/xxHash/Bin/" .. outputFolderName

	files
	{
		"Src/Prism-Core/**",
		"Src/pcpch.h",
		"Src/pcpch.cpp"
	}

	includedirs
	{
		"Src",

		includeDirs["glm"],
		includeDirs["spdlog"],
		includeDirs["xxHash"]
	}

	libdirs
	{
		libDirs["spdlog"],
		libDirs["xxHash"]
	}

	links
	{
		"spdlog.lib",
		"xxHash.lib"
	}

	filter "system:windows"
		systemversion "latest"
		defines { "PE_PLATFORM_WINDOWS" }

		files
		{
			"Src/Platform/SDL/**",
			"Src/RenderAPI/D3D12/**"
		}

		includedirs
		{
			includeDirs["SDL"]
		}

		libdirs
		{
			libDirs["SDL"]
		}

		links
		{
			"dxgi.lib",
			"dxguid.lib",
			"d3d12.lib",
			"dxcompiler.lib",
			"SDL3.lib"
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
