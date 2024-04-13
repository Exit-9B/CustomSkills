cmake_minimum_required(VERSION 3.24)

project(
	CustomSkills
	VERSION 3.0.0
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

if(SkyrimSE_PATH)
	Papyrus_Add(
		"PapyrusAPI"
		GAME ${SkyrimSE_PATH}
		IMPORTS
			"${PROJECT_SOURCE_DIR}/scripts"
		SOURCES
			"${PROJECT_SOURCE_DIR}/scripts/CustomSkills.psc"
		OPTIMIZE
		ANONYMIZE
		COMPONENT ${CMAKE_INSTALL_DEFAULT_COMPONENT_NAME}
	)
endif()

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
