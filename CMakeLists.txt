# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("wite")

set(CMAKE_CXX_STANDARD 20)

# Include sub-projects.
add_subdirectory("test")
add_subdirectory("examples")