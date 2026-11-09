add_library(Runtime INTERFACE)
add_library(${CMAKE_PROJECT_NAME}::Runtime ALIAS Runtime)

add_subdirectory(Core)

target_link_libraries(Runtime INTERFACE
	${CMAKE_PROJECT_NAME}::Core
)
