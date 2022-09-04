# Copyright 2020-2022 Nikita Fediuchin. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.16)

if (${CMAKE_SOURCE_DIR} MATCHES ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "Prevented in-tree build")
endif ()

if (TARGET uran-static)
	return()
endif ()

project(uran
	VERSION 0.0.1
	DESCRIPTION "Open multi-platform game and application engine"
	HOMEPAGE_URL "https://github.com/cfnptr/uran"
	LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	enable_language(OBJC)
endif ()

option(URAN_BUILD_SHARED "Build Uran shared library" ON)
option(URAN_BUILD_EXECUTABLE "Build Uran executable program" ON)

if (URAN_BUILD_EXECUTABLE)
	if (NOT DEFINED URAN_PROGRAM_NAME)
		set(URAN_PROGRAM_NAME uran-editor)
	endif ()
	if (NOT DEFINED URAN_PROGRAM_SOURCES)
		set(URAN_PROGRAM_SOURCES source/main.c)
	endif ()

	if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		if (NOT DEFINED URAN_MACOS_INFO_PLIST)
			set(URAN_MACOS_INFO_PLIST 
				${PROJECT_SOURCE_DIR}/cmake/MacOSXBundleInfo.plist.in)
		endif ()
		if (NOT DEFINED URAN_MACOS_RESOURCES)
			set(URAN_MACOS_RESOURCES
				resources/resources.pack resources/uran-editor.icns)
		endif ()
	endif ()
endif ()

if (URAN_BUILD_SHARED)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif ()

set(CONF_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(CONF_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(libraries/conf)

set(CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz ON CACHE BOOL "" FORCE)
add_subdirectory(libraries/freetype)

set(WEBP_BUILD_ANIM_UTILS OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_CWEBP OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_DWEBP OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_GIF2WEBP OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_IMG2WEBP OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_VWEBP OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_WEBPINFO OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_LIBWEBPMUX OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_WEBPMUX OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
add_subdirectory(libraries/libwebp)

set(LOGY_BUILD_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory(libraries/logy)

set(MPGX_BUILD_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory(libraries/mpgx)

set(PACK_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(PACK_BUILD_UTILITIES OFF CACHE BOOL "" FORCE)
add_subdirectory(libraries/pack)

set(MPNW_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(MPNW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(libraries/mpnw)

set(URAN_SOURCES
	source/pipelines/panel_pipeline.c

	source/renderers/panel_renderer.c
	source/renderers/text_renderer.c

	source/engine.c
	source/free_camera.c
	source/graphics_renderer.c
	source/image_data.c
	source/interface.c
	source/shader_data.c
	source/text.c
	source/transformer.c
	source/user_interface.c)
set(URAN_LINK_LIBRARIES
	conf-static
	freetype
	webp
	logy-static
	mpgx-static
	pack-static
	mpnw-static
	${URAN_PROGRAM_LIBRARIES})

if (NOT ${CMAKE_BUILD_TYPE} MATCHES "Release")
	list(APPEND URAN_SOURCES source/editor.c)
endif ()

configure_file(cmake/defines.h.in include/uran/defines.h)

add_library(uran-static STATIC ${URAN_SOURCES})
target_link_libraries(uran-static PUBLIC
	${URAN_LINK_LIBRARIES})
target_include_directories(uran-static PUBLIC
	${PROJECT_BINARY_DIR}/include
	${PROJECT_SOURCE_DIR}/include)

if (URAN_BUILD_SHARED)
	add_library(uran-shared SHARED ${URAN_SOURCES})
	set_target_properties(uran-shared PROPERTIES
		OUTPUT_NAME "uran"
		WINDOWS_EXPORT_ALL_SYMBOLS ON)
	target_link_libraries(uran-shared PUBLIC
		${URAN_LINK_LIBRARIES})
	target_include_directories(uran-shared PUBLIC
		${PROJECT_BINARY_DIR}/include
		${PROJECT_SOURCE_DIR}/include)
endif ()

if (URAN_BUILD_EXECUTABLE)
	if (${CMAKE_SYSTEM_NAME} MATCHES "Windows" AND
		(${CMAKE_BUILD_TYPE} MATCHES "Release" OR
		${CMAKE_BUILD_TYPE} MATCHES "RelWithDebInfo" OR
		${CMAKE_BUILD_TYPE} MATCHES "MinSizeRel"))
		add_executable(${URAN_PROGRAM_NAME} WIN32 ${URAN_PROGRAM_SOURCES})
	elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		add_executable(${URAN_PROGRAM_NAME}
			${URAN_PROGRAM_SOURCES} ${URAN_MACOS_RESOURCES})
	else ()
		add_executable(${URAN_PROGRAM_NAME} ${URAN_PROGRAM_SOURCES})
	endif ()

	if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		set_target_properties(${URAN_PROGRAM_NAME} PROPERTIES
			MACOSX_BUNDLE TRUE
			MACOSX_BUNDLE_INFO_PLIST ${URAN_MACOS_INFO_PLIST}
			RESOURCE "${URAN_MACOS_RESOURCES}")

		install(CODE "set(PROJECT_SOURCE_DIR \"${PROJECT_SOURCE_DIR}\")" COMPONENT FixupBundle)
		install(CODE "set(URAN_APP_PATH \"${PROJECT_BINARY_DIR}/${URAN_PROGRAM_NAME}.app\")" COMPONENT FixupBundle)

		install(CODE [[
			configure_file(/usr/local/lib/libMoltenVK.dylib
				${URAN_APP_PATH}/Contents/Frameworks/libMoltenVK.dylib COPYONLY)
			configure_file(${PROJECT_SOURCE_DIR}/resources/MoltenVK_icd.json
				${URAN_APP_PATH}/Contents/Resources/vulkan/icd.d/MoltenVK_icd.json COPYONLY)

			include(BundleUtilities)
			fixup_bundle("${URAN_APP_PATH}" "" "")
		]] COMPONENT FixupBundle)
	else ()
		configure_file(resources/resources.pack resources.pack COPYONLY)
	endif ()

	target_link_libraries(${URAN_PROGRAM_NAME} PRIVATE uran-static)
	target_include_directories(${URAN_PROGRAM_NAME} PRIVATE
		${PROJECT_BINARY_DIR}/include
		${PROJECT_SOURCE_DIR}/include
		${URAN_PROGRAM_INCLUDES})

	if (${CMAKE_BUILD_TYPE} MATCHES "Release" AND
		(${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR
		${CMAKE_SYSTEM_NAME} MATCHES "Darwin"))
		add_custom_command(TARGET ${URAN_PROGRAM_NAME} POST_BUILD
			COMMAND strip "$<TARGET_FILE:${URAN_PROGRAM_NAME}>"
			VERBATIM)
	endif ()
endif ()
