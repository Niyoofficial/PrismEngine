project "WinPixEventRuntime"
	kind "Utility"

	files
	{
		"Include/**"
	}

	targetdir ("bin/x64/")

	postbuildcommands
	{
		"{COPY} %{cfg.buildtarget.abspath}.lib " .. binDirectoryProject,
		"{COPY} %{cfg.buildtarget.abspath}.dll " .. binDirectoryRoot .. "LearnD3D12/"
	}
