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

cmake_minimum_required(VERSION 3.10)

if (${CMAKE_SOURCE_DIR} MATCHES ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "Prevented in-tree build")
endif ()

if (TARGET ran)
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

option(URAN_BUILD_SHARED "Build Uran shared library" ON)
option(URAN_BUILD_EDITOR "Build Uran editor executable" ON)

set(CONF_BUILD_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory(libraries/conf)

set(LOGY_BUILD_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory(libraries/logy)

set(MPGX_BUILD_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory(libraries/mpgx)

set(PACK_BUILD_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory(libraries/pack)

set(URAN_SOURCES
	source/renderers/color_renderer.c
	source/renderers/diffuse_renderer.c
	source/renderers/simple_shadow_renderer.c
	source/renderers/sprite_renderer.c
	source/renderers/texture_color_renderer.c
	source/renderers/texture_sprite_renderer.c
	source/renderers/text_renderer.c

	source/editor.c
	source/free_camera.c
	source/graphics_renderer.c
	source/interface.c
	source/transformer.c)

set(URAN_LINK_LIBRARIES
	conf-static logy-static mpgx-static pack-static)

configure_file(
	cmake/defines.h.in
	uran/defines.h)

add_library(uran-static STATIC
	${URAN_SOURCES})
set_target_properties(uran-static PROPERTIES
	OUTPUT_NAME "uran")
target_link_libraries(uran-static PUBLIC
	${URAN_LINK_LIBRARIES})
target_include_directories(uran-static PUBLIC
	${PROJECT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/include)

if (URAN_BUILD_SHARED)
	add_library(uran-shared SHARED
		${URAN_SOURCES})
	set_target_properties(uran-shared PROPERTIES
		OUTPUT_NAME "uran"
		WINDOWS_EXPORT_ALL_SYMBOLS ON)
	target_link_libraries(uran-shared PUBLIC
		${URAN_LINK_LIBRARIES})
	target_include_directories(uran-shared PUBLIC
		${PROJECT_BINARY_DIR}
		${PROJECT_SOURCE_DIR}/include)
endif ()

if (URAN_BUILD_EDITOR)
	if (${CMAKE_SYSTEM_NAME} MATCHES "Windows" AND
		${CMAKE_BUILD_TYPE} MATCHES "Release")
		add_executable(uran-editor WIN32 source/main.c)
	else ()
		add_executable(uran-editor source/main.c)
	endif ()

	set_target_properties(uran-editor PROPERTIES
		OUTPUT_NAME "uran-editor")
	target_link_libraries(uran-editor PRIVATE
		uran-static)
	target_include_directories(uran-editor PRIVATE
		${PROJECT_BINARY_DIR}
		${PROJECT_SOURCE_DIR}/include
		${PROJECT_SOURCE_DIR}/program)

	if (${CMAKE_BUILD_TYPE} MATCHES "Release" AND
		(${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR
		${CMAKE_SYSTEM_NAME} MATCHES "Darwin"))
		add_custom_command(TARGET uran-editor POST_BUILD
			COMMAND strip "$<TARGET_FILE:uran-editor>"
			VERBATIM)
	endif ()
endif ()
