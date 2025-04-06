cmake_minimum_required(VERSION 3.26)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(PrismEngine)

add_subdirectory(PrismCore)
add_subdirectory(Sandbox)
