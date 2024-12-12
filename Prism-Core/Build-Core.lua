project "Prism-Core"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	pchheader "pcpch.h"
	pchsource "Src/pcpch.cpp"

	includeDirs["SDL"] = path.normalize("%{prj.location}/Vendor/SDL/include")
	libDirs["SDL"] = path.normalize("%{prj.location}/Vendor/SDL/Bin/" .. outputFolderName)
	includeDirs["DirectXTK12"] = path.normalize("%{prj.location}/Vendor/DirectXTK12/Inc")
	libDirs["DirectXTK12"] = path.normalize("%{prj.location}/Vendor/DirectXTK12/Bin/" .. outputFolderName)
	includeDirs["assimp"] = path.normalize("%{prj.location}/Vendor/assimp/include")
	libDirs["assimp"] = path.normalize("%{prj.location}/Vendor/assimp/Bin/" .. outputFolderName)
	includeDirs["stb"] = path.normalize("%{prj.location}/Vendor/stb/")

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
		includeDirs["spdlog-fmt"],
		includeDirs["assimp"],
		includeDirs["stb"],
		includeDirs["AgilitySDK"],
		includeDirs["boost"]
	}

	libdirs
	{	
		libDirs["spdlog"],
		libDirs["spdlog-fmt"],
		libDirs["assimp"]
	}

	links
	{
		"spdlog.lib",
		"fmt.lib",
		"assimp.lib",
		"zlibstatic.lib"
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
			includeDirs["SDL"],
			includeDirs["DirectXTK12"],
			includeDirs["WinPixEventRuntime"],
			includeDirs["AgilitySDK"]
		}

		libdirs
		{
			libDirs["SDL"],
			libDirs["DirectXTK12"],
			libDirs["WinPixEventRuntime"],
		}

		links
		{
			"dxgi.lib",
			"dxguid.lib",
			"d3d12.lib",
			"dxcompiler.lib",
			"SDL3.lib",
			"WinPixEventRuntime.lib",
			"DirectXTK12.lib"
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
