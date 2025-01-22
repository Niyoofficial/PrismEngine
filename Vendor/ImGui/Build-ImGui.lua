project "ImGui"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	files
	{
		"misc/cpp/imgui_stdlib.h",
		"misc/cpp/imgui_stdlib.cpp",
		"imconfig.h",
		"imgui.cpp",
		"imgui.h",
		"imgui_demo.cpp",
		"imgui_draw.cpp",
		"imgui_internal.h",
		"imgui_tables.cpp",
		"imgui_widgets.cpp",
		"imstb_rectpack.h",
		"imstb_textedit.h",
		"imstb_truetype.h",
		"misc/debuggers/imgui.natvis",
		"misc/debuggers/imgui.natstepfilter"
	}

	links
	{
		"SDL3.lib",
	}

	includedirs
	{
		_SCRIPT_DIR,
		path.normalize("../../Prism-Core/Vendor/SDL/Include")
	}

	libdirs
	{
		path.normalize("../../Prism-Core/Vendor/SDL/Bin/" .. outputFolderName)
	}

	dependson
	{
		"SDL"
	}

	filter "system:windows"
		systemversion "latest"

	files
	{
		"backends/imgui_impl_dx12.h",
		"backends/imgui_impl_dx12.cpp",
		"backends/imgui_impl_sdl3.h",
		"backends/imgui_impl_sdl3.cpp",
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
