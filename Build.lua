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

	libDirs = {}
	libDirs["spdlog"] = path.normalize("%{wks.location}/Vendor/spdlog/Bin/" .. outputFolderName)

	filter "system:windows"
		libDirs["dxc"] = path.normalize("%{wks.location}/Vendor/dxc/")
		includeDirs["WinPixEventRuntime"] = path.normalize("%{wks.location}/Vendor/WinPixEventRuntime/Include/")
		libDirs["WinPixEventRuntime"] = path.normalize("%{wks.location}/Vendor/WinPixEventRuntime/bin/x64/")

		defines { "USE_PIX" }
	filter {}

group "Dependencies"
	include "Vendor/glm/Build-glm.lua"
	include "Vendor/spdlog/Build-spdlog.lua"
group ""

group "Core"
	include "Prism-Core/Build-Core.lua"
group "Core/Dependencies"
	include "Prism-Core/Vendor/SDL/Build-SDL.lua"
	include "Prism-Core/Vendor/xxHash/Build-xxHash.lua"
group ""

include "Prism-Sandbox/Build-Sandbox.lua"
