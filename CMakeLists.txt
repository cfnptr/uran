# Copyright 2022 Nikita Fediuchin. All rights reserved.
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

project(mpgx 
	VERSION 0.0.1
	DESCRIPTION "Multi-platform graphics library"
	HOMEPAGE_URL "https://github.com/cfnptr/mpgx"
	LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_subdirectory(libraries/mpgx)

set(URAN_SOURCES
	source/main.c)

set(MPGX_LINK_LIBRARIES
	mpgx)

configure_file(
	cmake/defines.h.in
	uran/defines.h)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows" AND
	${CMAKE_BUILD_TYPE} MATCHES "Release")
	add_executable(uran-editor WIN32
		${URAN_SOURCES})
else ()
	add_executable(uran-editor
		${URAN_SOURCES})
endif ()

set_target_properties(uran-editor PROPERTIES
	OUTPUT_NAME "uran-editor")
target_link_libraries(uran-editor PRIVATE
	${URAN_LINK_LIBRARIES})
target_include_directories(uran-editor PRIVATE
	${PROJECT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/include)

if (${CMAKE_BUILD_TYPE} MATCHES "Release" AND
	(${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR
	${CMAKE_SYSTEM_NAME} MATCHES "Darwin"))
	add_custom_command(TARGET uran-editor POST_BUILD
		COMMAND strip "$<TARGET_FILE:uran-editor>"
		VERBATIM)
endif ()
