project(PrismManagedHost LANGUAGES CSharp)

file(GLOB_RECURSE SOURCES
	*
)

# Set build action for all .axaml files to AvaloniaXaml
set(AXAML_FILES ${SOURCES})
list(FILTER AXAML_FILES INCLUDE REGEX "\\.axaml$")
set_source_files_properties(${AXAML_FILES} PROPERTIES VS_TOOL_OVERRIDE "AvaloniaXaml")

add_library(${PROJECT_NAME} SHARED ${SOURCES})
add_library(${CMAKE_PROJECT_NAME}::ManagedHost ALIAS ${PROJECT_NAME})


set_target_properties(${PROJECT_NAME} PROPERTIES
    DOTNET_SDK "Microsoft.NET.Sdk"
    DOTNET_TARGET_FRAMEWORK "net10.0"
    VS_GLOBAL_GenerateRuntimeConfigurationFiles "true"
	VS_GLOBAL_CopyLocalLockFileAssemblies "true"
	VS_GLOBAL_AllowUnsafeBlocks "true"
)

set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY VS_PACKAGE_REFERENCES 
	"Avalonia_12.1.1"
	"Avalonia.Desktop_12.1.1"
	"Avalonia.Themes.Fluent_12.1.1"
	"Avalonia.Fonts.Inter_12.1.1"
	"CommunityToolkit.Mvvm_8.4.2"
)
