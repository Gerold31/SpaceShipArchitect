cmake_minimum_required(VERSION 3.1)
project( SOIL )

# Create some variables
set(SOURCE_FILES
	"src/image_DXT.c"
	"src/image_helper.c"
	"src/SOIL.c"
	"src/stb_image_aug.c")
set(HEADER_FILES
	"src/image_DXT.h"
	"src/image_helper.h"
	"src/SOIL.h"
	"src/stb_image_aug.h"
	"src/stbi_DDS_aug.h"
	"src/stbi_DDS_aug_c.h")

# Create source groups (for Visual Studio)
source_group("Headers" FILES ${HEADER_FILES})
source_group("Sources" FILES ${SOURCE_FILES})

# Add binaries
add_library("${PROJECT_NAME}" ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories("${PROJECT_NAME}" PUBLIC "src/")

# Use C++11
set_target_properties("${PROJECT_NAME}" PROPERTIES LINKER_LANGUAGE C)
set_target_properties("${PROJECT_NAME}" PROPERTIES C_STANDARD 11)
