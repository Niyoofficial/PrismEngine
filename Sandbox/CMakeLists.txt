cmake_minimum_required(VERSION 3.26)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Sandbox)

file(GLOB_RECURSE SOURCES
	src/*
)

add_executable(${PROJECT_NAME} 
		${SOURCES})

target_include_directories(${PROJECT_NAME}
	PUBLIC
		${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PUBLIC PrismEngine::Core)

set_property(TARGET Sandbox PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
