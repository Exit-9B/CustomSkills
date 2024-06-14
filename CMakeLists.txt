cmake_minimum_required(VERSION 3.24)

project(
	CustomSkills
	VERSION 3.1.0
	LANGUAGES CXX
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

list(APPEND CMAKE_IGNORE_PATH "${PROJECT_BINARY_DIR}")
list(APPEND CMAKE_MODULE_PATH
	"${PROJECT_SOURCE_DIR}/tools/SKSE-CMakeModules/Modules"
	"${PROJECT_SOURCE_DIR}/tools/BethesdaCMakeModules/Modules"
)
include(SKSEPlugin)
include(Papyrus)

set(SKSE_USE_XBYAK ON)
set(SKSE_SUPPORT_VR OFF)

SKSEPlugin_Add(
	${PROJECT_NAME}
	INCLUDE_DIR include
	SOURCE_DIR src
	SOURCES
		.clang-format
		.editorconfig
		README.md
		vcpkg.json
	PRECOMPILE_HEADERS
		src/PCH/PCH.h
)

find_path(SIMPLEINI_INCLUDE_DIRS "SimpleIni.h")
find_package(jsoncpp CONFIG REQUIRED)

target_include_directories(
	${PROJECT_NAME}
	PRIVATE
		${SIMPLEINI_INCLUDE_DIRS}
)

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
		jsoncpp_static
)

file(DOWNLOAD
	https://github.com/ianpatt/skse64/releases/download/v2.2.6/skse64_2_02_06.7z
	"${CMAKE_CURRENT_BINARY_DIR}/download/skse64_2_02_06.7z"
)

file(ARCHIVE_EXTRACT
	INPUT "${CMAKE_CURRENT_BINARY_DIR}/download/skse64_2_02_06.7z"
	DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/tools"
)

file(GLOB SCRIPT_SOURCES
	LIST_DIRECTORIES false
	CONFIGURE_DEPENDS
	"${PROJECT_SOURCE_DIR}/scripts/*.psc"
)

Papyrus_Add(
	"PapyrusAPI"
	#GAME ${SkyrimSE_PATH}
	MODE SkyrimSE
	IMPORTS
		"${PROJECT_SOURCE_DIR}/scripts"
		"${CMAKE_CURRENT_BINARY_DIR}/tools/skse64_2_02_06/Data/Scripts/Source"
	SOURCES ${SCRIPT_SOURCES}
	COMPONENT ${CMAKE_INSTALL_DEFAULT_COMPONENT_NAME}
	ANONYMIZE
	SKIP_DEFAULT_IMPORTS
)

install(
	FILES
		"${PROJECT_SOURCE_DIR}/data/SKSE/CustomConsole/CustomSkills.yaml"
	DESTINATION
		SKSE/CustomConsole
)

set(CPACK_GENERATOR "7Z")
set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME}-${PROJECT_VERSION})
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY FALSE)
include(CPack)
