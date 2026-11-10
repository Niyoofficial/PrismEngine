project(Sandbox)

file(GLOB_RECURSE SOURCES
	src/*
	app.manifest
)

add_executable(${PROJECT_NAME} 
		${SOURCES})

target_include_directories(${PROJECT_NAME}
	PUBLIC
		${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PUBLIC PrismEngine::Runtime)

set_property(TARGET Sandbox PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")

add_custom_command(
	TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E echo "start $<TARGET_FILE:${PROJECT_NAME}> -wo ${PROJECT_SOURCE_DIR} " > "${PROJECT_SOURCE_DIR}/Run${PROJECT_NAME}-$<CONFIG>.bat"
)
