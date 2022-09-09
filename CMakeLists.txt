# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("wite")

if(MSVC)
else()
add_compile_options(-Wc++2b-extensions)
endif()

# Include sub-projects.
add_subdirectory("test")
add_subdirectory("examples")