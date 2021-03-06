cmake_minimum_required(VERSION 3.1)
project( SpaceShipArchitect )

set(EXECUTABLE "SSA" CACHE STRING "Name of the resulting executable.")
set(VERSION_MAJOR 0  CACHE STRING "Project major version number.")
set(VERSION_MINOR 1  CACHE STRING "Project minor version number.")
mark_as_advanced(VERSION_MAJOR VERSION_MINOR)

## Change default build type to 'Debug'
if (NOT CMAKE_BUILD_TYPE)
	set(
		CMAKE_BUILD_TYPE "Debug" CACHE STRING
		"Build type, options: None, Debug, Release, RelWithDebInfo, MinSizeRel."
		FORCE
	)
endif()

## Load packages for cmake
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package(Doxygen)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)

## Add libraries distributed with the repository
add_subdirectory("libs")

## Create some variables to use them below
set(SOURCE_DIR "src")
set(INCLUDE_DIR "include")
set(RESOURCE_DIR "resources")

file(GLOB_RECURSE SOURCE_FILES
	"${SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE HEADER_FILES
	"${INCLUDE_DIR}/*.h" "${INCLUDE_DIR}/*.hpp")
file(GLOB_RECURSE RESOURCE_FILES
	"${RESOURCE_DIR}/*")

## Workaround to show resources in IDE
add_custom_target(Resources ALL SOURCES ${RESOURCE_FILES})

## Create source groups (for Visual Studio)
source_group("Headers" FILES ${HEADER_FILES})
source_group("Sources" FILES ${SOURCE_FILES})
source_group("Resources" FILES ${RESOURCE_FILES})

## Add executable
add_executable("${EXECUTABLE}" ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories("${EXECUTABLE}" PUBLIC "${INCLUDE_DIR}/")

## Link with libraries
target_link_libraries("${EXECUTABLE}" glm glfw gtl SOIL utl)
target_include_directories("${EXECUTABLE}" PRIVATE ${OpenGL_INCLUDE_DIRS})
target_include_directories("${EXECUTABLE}" PRIVATE ${GLEW_INCLUDE_DIRS})
target_link_libraries("${EXECUTABLE}" ${OPENGL_LIBRARIES})
target_link_libraries("${EXECUTABLE}" ${GLEW_LIBRARIES})

## Use C++11
set_target_properties("${EXECUTABLE}" PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties("${EXECUTABLE}" PROPERTIES CXX_STANDARD 11)
set_target_properties("${EXECUTABLE}" PROPERTIES CXX_STANDARD_REQUIRED ON)

## Create header with build information
configure_file(
	"${PROJECT_SOURCE_DIR}/config.h.in"
	"${PROJECT_BINARY_DIR}/config.h" ESCAPE_QUOTES)
include_directories("${PROJECT_BINARY_DIR}")

## Add rule for Doxygen
option(DOC_TARGET
	"Add a target to create the HTML based documentation (requires Doxygen)"
	${DOXYGEN_FOUND})

if (DOC_TARGET)
	if (NOT DOXYGEN_FOUND)
		message(FATAL_ERROR "Doxygen not found")
	endif()

	configure_file(
		"${PROJECT_SOURCE_DIR}/Doxyfile.in"
		"${PROJECT_BINARY_DIR}/Doxyfile" ESCAPE_QUOTES @ONLY)
	add_custom_target(doc
		"${DOXYGEN_EXECUTABLE}" "${PROJECT_BINARY_DIR}/Doxyfile"
		WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
		COMMENT "Generating API documentation with Doxygen" VERBATIM)
endif()

## Specify install rules
install(TARGETS "${EXECUTABLE}"
	DESTINATION "bin")
install(DIRECTORY "${RESOURCE_DIR}/"
	DESTINATION "res")
