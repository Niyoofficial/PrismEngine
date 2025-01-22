workspace "PrismEngine"
	architecture "x64"
	configurations { "Debug", "Profile", "Release" }
	startproject "Prism-Sandbox"

	-- Workspace-wide build options for MSVC
	filter "system:windows"
		buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }
	filter {}

	outputFolderName = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
	binDirectory = path.normalize("%{prj.location}/Bin/" .. outputFolderName .. "/")
	intDirectory = path.normalize("%{prj.location}/Bin/" .. outputFolderName .. "/Int/")

	targetdir (binDirectory)
	objdir (intDirectory)

	includeDirs = {}
	includeDirs["Prism-Core"] = path.normalize("%{wks.location}/Prism-Core/Src")
	includeDirs["glm"] = path.normalize("%{wks.location}/Vendor/glm")
	includeDirs["spdlog"] = path.normalize("%{wks.location}/Vendor/spdlog/include")
	includeDirs["spdlog-fmt"] = path.normalize("%{wks.location}/Vendor/spdlog/dependencies/fmt-src/include")
	includeDirs["boost"] = path.normalize("%{wks.location}/Vendor/boost_1_82")
	includeDirs["ImGui"] = path.normalize("%{wks.location}/Vendor/ImGui")

	libDirs = {}
	libDirs["spdlog"] = path.normalize("%{wks.location}/Vendor/spdlog/Bin/" .. outputFolderName)
	libDirs["spdlog-fmt"] = path.normalize("%{wks.location}/Vendor/spdlog/Bin/" .. outputFolderName)
	libDirs["ImGui"] = path.normalize("%{wks.location}/Vendor/ImGui/Bin/" .. outputFolderName)

	defines
	{
		"GLM_FORCE_DEPTH_ZERO_TO_ONE",
		"GLM_FORCE_LEFT_HANDED"
	}

	includedirs
	{
		includeDirs["glm"],
		includeDirs["ImGui"],
		includeDirs["spdlog"],
		includeDirs["spdlog-fmt"]
	}

	libdirs
	{	
		libDirs["ImGui"],
		libDirs["spdlog"],
		libDirs["spdlog-fmt"]
	}

	links
	{
		"ImGui.lib",
		"spdlog.lib",
		"fmt.lib"
	}

	filter "system:windows"
		includeDirs["dxc"] = path.normalize("%{wks.location}/Vendor/dxc/inc/")
		libDirs["dxc"] = path.normalize("%{wks.location}/Vendor/dxc/bin/x64/")
		includeDirs["WinPixEventRuntime"] = path.normalize("%{wks.location}/Vendor/WinPixEventRuntime/Include/")
		libDirs["WinPixEventRuntime"] = path.normalize("%{wks.location}/Vendor/WinPixEventRuntime/bin/x64/")
		includeDirs["AgilitySDK"] = path.normalize("%{wks.location}/Vendor/AgilitySDK/include/")
		libDirs["AgilitySDK"] = path.normalize("%{wks.location}/Vendor/AgilitySDK/bin/x64/")

		defines { "USE_PIX" }
	filter {}
	
	group "Core"
		include "Prism-Core/Build-Core.lua"
	group ""

	include "Prism-Sandbox/Build-Sandbox.lua"

	group "Dependencies"
		include "Vendor/glm/Build-glm.lua"
		include "Vendor/spdlog/Build-spdlog.lua"
		include "Vendor/boost_1_82/Build-boost.lua"
		include "Vendor/ImGui/Build-ImGui.lua"

		filter "system:windows"
			include "Vendor/AgilitySDK/Build-AgilitySDK.lua"
			include "Vendor/dxc/Build-dxc.lua"
			include "Vendor/WinPixEventRuntime/Build-WinPixEventRuntime.lua"
		filter {}
	group ""
