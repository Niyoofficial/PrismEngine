project(PrismCore LANGUAGES CXX)

option(PE_RENDER_BACKEND_D3D12 "Enable DirectX 12 render backend" ON)
option(PE_RENDER_BACKEND_VULKAN "Enable Vulkan render backend" OFF)
option(PE_PLATFORM_LAYER_SDL "Enable SDL platform layer" ON)
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_SUPERLUMINAL_API "Enable Superluminal API integration" ON)

if (PE_RENDER_BACKEND_D3D12 AND PE_RENDER_BACKEND_VULKAN)
	message(FATAL_ERROR
			"Only one Render Backend can be enabled. "
			"PE_RENDER_BACKEND_D3D12 and PE_RENDER_BACKEND_VULKAN cannot both be ON."
	)
elseif (NOT PE_RENDER_BACKEND_D3D12 AND NOT PE_RENDER_BACKEND_VULKAN)
	message(FATAL_ERROR
			"No Render Backend is enabled. "
			"Enable either PE_RENDER_BACKEND_D3D12 or PE_RENDER_BACKEND_VULKAN."
	)
endif ()

if (NOT PE_PLATFORM_LAYER_SDL)
	message(FATAL_ERROR
			"SDL platform layer is required. "
			"PE_PLATFORM_LAYER_SDL must be ON."
	)
endif ()

file(GLOB_RECURSE SOURCES
		Prism/*
		pcpch.h
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
add_library(${CMAKE_PROJECT_NAME}::Core ALIAS ${PROJECT_NAME})

if (WIN32)
	target_compile_options(${PROJECT_NAME} PUBLIC "$<$<C_COMPILER_ID:MSVC>:/utf-8>")
	target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
	target_compile_options(${PROJECT_NAME} PUBLIC "/Zc:preprocessor")
	target_compile_definitions(${PROJECT_NAME} PUBLIC "PE_PLATFORM_WINDOWS=1")

	add_definitions(-DUNICODE -D_UNICODE)

	find_package(winpixevent CONFIG REQUIRED)
	target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::WinPixEventRuntime)
endif ()

if (ENABLE_ASAN)
	if (MSVC)
		target_compile_options(${PROJECT_NAME} PUBLIC /fsanitize=address /Zi)

		target_compile_definitions(${PROJECT_NAME} PUBLIC _DISABLE_STRING_ANNOTATION _DISABLE_VECTOR_ANNOTATION)
	else ()
		target_compile_options(${PROJECT_NAME} PUBLIC -fsanitize=address -fno-omit-frame-pointer -g -O1)

		target_link_options(${PROJECT_NAME} PUBLIC -fsanitize=address)
	endif ()
endif ()

target_precompile_headers(${PROJECT_NAME} PRIVATE pcpch.h)

target_include_directories(${PROJECT_NAME}
		PUBLIC
		${PROJECT_SOURCE_DIR}
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
		$<$<CONFIG:Debug>:PE_BUILD_DEBUG=1>
		$<$<CONFIG:RelWithDebInfo>:PE_BUILD_PROFILE=1>
		$<$<CONFIG:Release>:PE_BUILD_RELEASE=1>
)

target_compile_definitions(${PROJECT_NAME} PRIVATE PE_ENGINE_DIR=${PE_ENGINE_DIRECTORY})
message(STATUS "Engine directory set to: ${PE_ENGINE_DIRECTORY}")

if (PE_PLATFORM_LAYER_SDL)
	file(GLOB_RECURSE SDL_SOURCES "Platform/SDL/*")
	target_sources(${PROJECT_NAME} PRIVATE ${SDL_SOURCES})
endif ()

if (PE_RENDER_BACKEND_D3D12)
	target_compile_definitions(${PROJECT_NAME} PUBLIC "PE_SUPPORT_D3D12=1")
	target_compile_definitions(${PROJECT_NAME} PUBLIC "USE_PIX=1")

	file(GLOB_RECURSE D3D12_SOURCES "RenderAPI/D3D12/*")
	target_sources(${PROJECT_NAME} PRIVATE ${D3D12_SOURCES})

	if (NOT "${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}" STREQUAL "")
		set(PE_WINDOWS_SDK_VERSION ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION})
	else ()
		set(PE_WINDOWS_SDK_VERSION ${CMAKE_SYSTEM_VERSION})
	endif ()

	find_library(D3D12_LIB NAMES d3d12 HINTS "$ENV{ProgramFiles\(x86\)}/Windows Kits/10/Lib/${PE_WINDOWS_SDK_VERSION}/um/x64")

	find_package(directxtk12 CONFIG REQUIRED)
	target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::DirectXTK12)

	find_package(directx-headers CONFIG REQUIRED)
	find_package(directx12-agility CONFIG REQUIRED)
	target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::DirectX-Headers Microsoft::DirectX-Guids Microsoft::DirectX12-Agility)

	if (TARGET Microsoft::DirectX12-Agility)
		file(MAKE_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/D3D12")
		add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PROPERTY:Microsoft::DirectX12-Core,IMPORTED_LOCATION_RELEASE> $<TARGET_FILE_DIR:Sandbox>/D3D12/D3D12Core.dll
				COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PROPERTY:Microsoft::DirectX12-Layers,IMPORTED_LOCATION_DEBUG> $<TARGET_FILE_DIR:Sandbox>/D3D12/D3D12SDKLayers.dll
				COMMAND_EXPAND_LISTS
		)
	endif ()

	target_link_libraries(${PROJECT_NAME} PRIVATE dxgi.lib)
	target_link_libraries(${PROJECT_NAME} PRIVATE dxguid.lib)
elseif (PE_RENDER_BACKEND_VULKAN)
	target_compile_definitions(${PROJECT_NAME} PUBLIC "PE_SUPPORT_VULKAN=1")

	file(GLOB_RECURSE VULKAN_SOURCES "RenderAPI/Vulkan/*")
	target_sources(${PROJECT_NAME} PRIVATE ${VULKAN_SOURCES})

	find_package(Vulkan REQUIRED)
	target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)

	find_package(VulkanMemoryAllocator CONFIG REQUIRED)
	target_link_libraries(${PROJECT_NAME} PRIVATE GPUOpen::VulkanMemoryAllocator)

	find_package(unofficial-spirv-reflect CONFIG REQUIRED)
	target_link_libraries(${PROJECT_NAME} PRIVATE unofficial::spirv-reflect)
endif ()

target_link_libraries(${PROJECT_NAME} PUBLIC PrismEngine::ManagedHost)
target_compile_definitions(${PROJECT_NAME} PUBLIC
	PE_MANAGED_HOST_BINARY_DIR=${CMAKE_BINARY_DIR}/engine/src/Runtime/ManagedHost/$<CONFIG>
)

find_package(glm CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)
target_compile_definitions(${PROJECT_NAME} PUBLIC "GLM_ENABLE_EXPERIMENTAL=1")
target_compile_definitions(${PROJECT_NAME} PUBLIC "GLM_FORCE_LEFT_HANDED=1")
target_compile_definitions(${PROJECT_NAME} PUBLIC "GLM_FORCE_DEPTH_ZERO_TO_ONE=1")

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC spdlog::spdlog)

find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)

find_package(imguizmo CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE imguizmo::imguizmo)

find_package(assimp CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE assimp::assimp)

find_package(xxHash CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE xxHash::xxhash)

find_package(yaml-cpp CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE yaml-cpp::yaml-cpp)

find_package(crossguid CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE crossguid)

find_package(directx-dxc CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::DirectXShaderCompiler)

find_package(unofficial-nethost CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE unofficial::nethost::nethost)

list(APPEND CMAKE_MODULE_PATH "${PE_ENGINE_DIRECTORY}/src/ThirdParty/DotNETHelpers")
find_package(DotNETHelpers REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${DotNETHelpers_INCLUDE_DIRS})

if (ENABLE_SUPERLUMINAL_API)
	list(APPEND CMAKE_MODULE_PATH "${PE_ENGINE_DIRECTORY}/src/ThirdParty/SuperluminalAPI")
	find_package(SuperluminalAPI REQUIRED)
	target_link_libraries(${PROJECT_NAME} PRIVATE SuperluminalAPI)
	target_compile_definitions(${PROJECT_NAME} PUBLIC "ENABLE_SUPERLUMINAL_API=1")
endif ()
