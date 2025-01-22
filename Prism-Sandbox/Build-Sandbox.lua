project "Prism-Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	files { "Src/**" }

	includedirs
	{
		"Src",

		includeDirs["Prism-Core"]
	}

	links
	{
		"Prism-Core"
	}

	filter "system:windows"
		systemversion "latest"
		defines { "PE_PLATFORM_WINDOWS" }

		includedirs
		{
			includeDirs["WinPixEventRuntime"]
		}

		libdirs
		{
			libDirs["WinPixEventRuntime"]
		}

		links
		{
			"Winmm.lib",
			"Version.lib",
			"Imm32.lib",
			"Cfgmgr32.lib",
			"WinPixEventRuntime.lib"
		}

		postbuildcommands
		{
			"{COPYFILE} %[%{libDirs.dxc}/dxcompiler.dll] %[%{binDirectory}dxcompiler.dll]",
			"{COPYFILE} %[%{libDirs.dxc}/dxil.dll] %[%{binDirectory}dxil.dll]",
			"{COPYFILE} %[%{libDirs.WinPixEventRuntime}/WinPixEventRuntime.dll] %[%{binDirectory}WinPixEventRuntime.dll]",
			"{MKDIR} %{binDirectory}D3D12",
			"{COPYFILE} %[%{libDirs.AgilitySDK}/D3D12Core.dll] %[%{binDirectory}D3D12/D3D12Core.dll]",
			"{COPYFILE} %[%{libDirs.AgilitySDK}/D3D12Core.pdb] %[%{binDirectory}D3D12/D3D12Core.pdb]",
			"{COPYFILE} %[%{libDirs.AgilitySDK}/d3d12SDKLayers.dll] %[%{binDirectory}D3D12/d3d12SDKLayers.dll]",
			"{COPYFILE} %[%{libDirs.AgilitySDK}/d3d12SDKLayers.pdb] %[%{binDirectory}D3D12/d3d12SDKLayers.pdb]"
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
