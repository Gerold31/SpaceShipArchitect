cmake_minimum_required(VERSION 3.1)
project( SpaceShipArchitect )

## Load packages for cmake
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

## Create some variables to use them below
set(INCLUDE_DIR "include/")
set(SOURCE_DIR "src/")
file(GLOB_RECURSE SOURCE_FILES
	"src/*.cpp")
file(GLOB_RECURSE HEADER_FILES
	"include/*.h" "include/*.hpp")
#file(GLOB_RECURSE SHADER_FILES
#	"shader/*.glsl" "shader/*.vert" "shader/*.frag" "shader/*.geom"
#	"shader/*.tcs" "shader/*.tes")

## Show shaders in IDE
#add_custom_target(Shader ALL SOURCES ${SHADER_FILES})

## Create source groups (for Visual Studio)
source_group("Headers" FILES ${HEADER_FILES})
source_group("Sources" FILES ${SOURCE_FILES})
#source_group("Shaders" FILES ${SHADER_FILES})

## Copy shaders to build dir
#if (NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
#	foreach(item IN LISTS SHADER_FILES)
#		configure_file("${item}" "${item}" COPYONLY)
#	endforeach()
#endif()

## Add executable
add_executable("${PROJECT_NAME}" ${SOURCE_FILES} ${HEADER_FILES} ${SHADER})
target_include_directories("${PROJECT_NAME}" PUBLIC "${INCLUDE_DIR}")

## Add include directories of libraries
target_include_directories("${PROJECT_NAME}" PRIVATE ${OpenGL_INCLUDE_DIRS})
target_include_directories("${PROJECT_NAME}" PRIVATE ${GLEW_INCLUDE_DIRS})
target_include_directories("${PROJECT_NAME}" PRIVATE ${GLFW_INCLUDE_DIRS})

## Link with libraries
target_link_libraries("${PROJECT_NAME}" ${OPENGL_LIBRARIES})
target_link_libraries("${PROJECT_NAME}" ${GLEW_LIBRARIES})
target_link_libraries("${PROJECT_NAME}" ${GLFW_LIBRARIES})

## Use C++11
set_target_properties("${PROJECT_NAME}" PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties("${PROJECT_NAME}" PROPERTIES CXX_STANDARD 11)
