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
		"Src/pcpch.cpp",

		"Src/Platform/SDL/**"
	}

	includedirs
	{
		"Src",

		includeDirs["SDL"],
		includeDirs["assimp"],
		includeDirs["stb"],
		includeDirs["AgilitySDK"],
		includeDirs["boost"]
	}

	libdirs
	{	
		libDirs["SDL"],
		libDirs["assimp"]
	}

	links
	{
		"SDL3.lib",
		"assimp.lib",
		"zlibstatic.lib"
	}

	filter "system:windows"
		systemversion "latest"
		defines { "PE_PLATFORM_WINDOWS" }

		files
		{
			"Src/RenderAPI/D3D12/**"
		}

		includedirs
		{
			includeDirs["dxc"],
			includeDirs["DirectXTK12"],
			includeDirs["WinPixEventRuntime"],
			includeDirs["AgilitySDK"]
		}

		libdirs
		{
			libDirs["DirectXTK12"],
			libDirs["WinPixEventRuntime"],
		}

		links
		{
			"dxgi.lib",
			"dxguid.lib",
			"d3d12.lib",
			"dxcompiler.lib",
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

	group "Core/Dependencies"
		include "Prism-Core/Vendor/assimp/Build-assimp.lua"
		include "Prism-Core/Vendor/SDL/Build-SDL.lua"
		filter "system:windows"
			include "Prism-Core/Vendor/DirectXTK12/Build-DirectXTK12.lua"
			include "Prism-Core/Vendor/stb/Build-stb.lua"
		filter {}
	group ""
