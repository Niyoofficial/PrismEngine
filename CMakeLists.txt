cmake_minimum_required(VERSION 3.26)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(PrismEngine)

option(PE_BUILD_SAMPLES "Build sample projects" ON)

add_subdirectory(engine)

if (PE_BUILD_SAMPLES)
    add_subdirectory(samples)
endif()
