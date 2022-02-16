// Copyright 2020-2022 Nikita Fediuchin. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include "mpgx/window.h"
#include "pack/reader.h"
#include "logy/logger.h"

/*
 * Shader data structure.
 */
typedef struct ShaderData_T ShaderData_T;
/*
 * Shader data instance.
 */
typedef ShaderData_T* ShaderData;

/*
 * Create a new shader data instance from the file.
 * Returns shader data instance on success, otherwise NULL.
 *
 * filePath - shader data file path.
 * logger - logger instance or NULL.
 */
ShaderData createShaderDataFromFile(
	const char* filePath,
	Logger logger);
/*
 * Create a new shader data instance from the pack data.
 * Returns shader data instance on success, otherwise NULL.
 *
 * packReader - pack reader instance.
 * path - shader data item path.
 * logger - logger instance or NULL.
 */
ShaderData createShaderDataFromPack(
	PackReader packReader,
	const char* path,
	Logger logger);
/*
 * Destroys shader data instance.
 * shaderData - shader data instance or NULL.
 */
void destroyShaderData(ShaderData shaderData);

/*
 * Returns shader data code.
 * shaderData - shader data instance.
 */
const uint8_t* getShaderDataCode(ShaderData shaderData);
/*
 * Returns shader data size in bytes.
 * shaderData - shader data instance.
 */
size_t getShaderDataSize(ShaderData shaderData);

/*
 * Create a new shader instance from the code file.
 * Returns shader instance on success, otherwise NULL.
 *
 * filePath - shader file path.
 * window - window instance.
 * type - shader type.
 * logger - logger instance or NULL.
 */
Shader createShaderFromFile(
	const char* filePath,
	Window window,
	ShaderType type,
	Logger logger);
/*
 * Create a new shader instance from the pack code.
 * Returns shader instance on success, otherwise NULL.
 *
 * packReader - pack reader instance.
 * path - shader data item path.
 * window - window instance.
 * type - shader type.
 * logger - logger instance or NULL.
 */
Shader createShaderFromPack(
	PackReader packReader,
	const char* path,
	Window window,
	ShaderType type,
	Logger logger);
