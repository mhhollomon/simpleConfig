# ROOT

cmake_minimum_required(VERSION 3.16)


if (NOT DEFINED PROJECT_NAME)
    set(C5K_IS_SUBPROJECT OFF)
else()
    set(C5K_IS_SUBPROJECT ON)
endif()

project(simpleConfig VERSION 0.1.0
    DESCRIPTION "Schema driven configuaration parser/helper"
    LANGUAGES CXX)
    
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)


# Set the default build type to Release (if the user doesn't specify)
#
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
endif()
#
# Some options
#
option(BUILD_TEST "Enable tests" ON)

#
# Make sure we use -std=c++17 or higher
#
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_compile_options("$<$<CONFIG:DEBUG>:/Wall>")
    add_compile_options("$<$<CONFIG:CI>:/Wall>")
    add_compile_options("$<$<CONFIG:RELEASE>:/O2>")
else()
    add_compile_options("$<$<CONFIG:DEBUG>:-O0;-Wall;-pedantic>")
    add_compile_options("$<$<CONFIG:CI>:-O1;-Wall;-pedantic>")
    add_compile_options("$<$<CONFIG:RELEASE>:-O3>")
endif()


add_subdirectory(lib)


#
# other projects we use.
#
add_subdirectory(extern)

#
# build tests
#
if (BUILD_TEST AND NOT C5K_IS_SUBPROJECT)
    enable_testing()
    add_subdirectory(tests)
endif()
