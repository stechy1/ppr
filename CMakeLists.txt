cmake_minimum_required (VERSION 3.3)
project (ppr)

find_package(OpenCL REQUIRED)
include_directories(${OpenCL_INCLUDE_DIRS})
link_directories(${OpenCL_LIBRARY})

add_executable (pprDemo main.cpp cl.hpp)
target_include_directories (pprDemo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries (pprDemo ${OpenCL_LIBRARY})