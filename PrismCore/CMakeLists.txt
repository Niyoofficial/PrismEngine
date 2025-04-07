cmake_minimum_required(VERSION 3.26)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(PrismCore)

file(GLOB_RECURSE SOURCES
	src/*
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
add_library(${CMAKE_PROJECT_NAME}::Core ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
	PUBLIC
		${PROJECT_SOURCE_DIR}/src
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
	$<$<CONFIG:Debug>:PE_BUILD_DEBUG=1>
	$<$<CONFIG:RelWithDebInfo>:PE_BUILD_PROFILE=1>
	$<$<CONFIG:Release>:PE_BUILD_RELEASE=1>
)

if (WIN32)
	target_compile_options(${PROJECT_NAME} PUBLIC "$<$<C_COMPILER_ID:MSVC>:/utf-8>")
	target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
	target_compile_options(${PROJECT_NAME} PUBLIC "/Zc:preprocessor")
	target_compile_definitions(${PROJECT_NAME} PUBLIC "PE_PLATFORM_WINDOWS")
	add_definitions(-DUNICODE -D_UNICODE)
endif()

target_precompile_headers(${PROJECT_NAME} PRIVATE src/pcpch.h)

find_package(glm CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)
target_compile_definitions(${PROJECT_NAME} PUBLIC "GLM_ENABLE_EXPERIMENTAL")
target_compile_definitions(${PROJECT_NAME} PUBLIC "GLM_FORCE_LEFT_HANDED")
target_compile_definitions(${PROJECT_NAME} PUBLIC "GLM_FORCE_DEPTH_ZERO_TO_ONE")

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC spdlog::spdlog)

find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)

find_package(assimp CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE assimp::assimp)

find_package(directxtk12 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::DirectXTK12)

find_package(directx-headers CONFIG REQUIRED)
find_package(directx12-agility CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::DirectX-Headers Microsoft::DirectX-Guids Microsoft::DirectX12-Agility)


if(TARGET Microsoft::DirectX12-Agility)
	file(MAKE_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/D3D12")
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PROPERTY:Microsoft::DirectX12-Core,IMPORTED_LOCATION_RELEASE> $<TARGET_FILE_DIR:Sandbox>/D3D12/D3D12Core.dll
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PROPERTY:Microsoft::DirectX12-Layers,IMPORTED_LOCATION_DEBUG> $<TARGET_FILE_DIR:Sandbox>/D3D12/D3D12SDKLayers.dll
		COMMAND_EXPAND_LISTS
	)
endif()

find_package(directx-dxc CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::DirectXShaderCompiler)

target_link_libraries(${PROJECT_NAME} PRIVATE dxgi.lib)

find_package(winpixevent CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::WinPixEventRuntime)
