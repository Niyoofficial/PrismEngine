project "DirectXTK12"
	kind "StaticLib"
	language "C++"
	cppdialect "C++14"
	staticruntime "off"

	pchheader "pch.h"
	pchsource "Src/pch.cpp"

	files
	{
		"Inc/**",
		"Src/**",
		"Audio/**",
	}

	includedirs
	{
		"Inc",
		"Src",
		"Src/Shaders/Compiled"
	}

	filter "system:windows"
		removefiles { "**/XboxDDSTextureLoader.*" }
	filter {}

	filter "files:Src/Shaders/**"
		buildaction "None"
	filter {}

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
