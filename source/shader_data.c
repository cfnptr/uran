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

#include "uran/shader_data.h"
#include "mpio/file.h"
#include <string.h>

struct ShaderData_T
{
	uint8_t* code;
	size_t size;
};

inline static bool getShaderDataFromFile(
	const char* path,
	Logger logger,
	uint8_t** code,
	size_t* size)
{
	assert(path);
	assert(code);
	assert(size);

	FILE* file = openFile(path, "rb");

	if (!file)
	{
		if (logger)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to open shader data file. "
				"(path: %s)", path);
		}
		return false;
	}

	int seekResult = seekFile(file, 0, SEEK_END);

	if (seekResult != 0)
	{
		if (logger)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to seek shader data file. "
				"(path: %s)", path);
		}
		closeFile(file);
		return false;
	}

	size_t fileSize = tellFile(file);

	if (fileSize == 0)
	{
		if (logger)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to tell shader data file. "
				"(path: %s)", path);
		}
		closeFile(file);
		return false;
	}

	seekResult = seekFile(file, 0, SEEK_SET);

	if (seekResult != 0)
	{
		if (logger)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to seek shader data file."
				"(path: %s)", path);
		}
		closeFile(file);
		return false;
	}

	uint8_t* shaderCode;
	size_t readSize;

	GraphicsAPI api = getGraphicsAPI();

	if (api == VULKAN_GRAPHICS_API)
	{
		shaderCode = malloc(fileSize * sizeof(uint8_t));

		if (!shaderCode)
		{
			closeFile(file);
			return false;
		}

		readSize = fread(
			shaderCode,
			sizeof(uint8_t),
			fileSize,
			file);
	}
	else if (api == OPENGL_GRAPHICS_API)
	{
		shaderCode = malloc((fileSize + 1) * sizeof(uint8_t));

		if (!shaderCode)
		{
			closeFile(file);
			return false;
		}

		readSize = fread(
			shaderCode,
			sizeof(uint8_t),
			fileSize,
			file);
		shaderCode[fileSize] = '\0';
	}
	else
	{
		abort();
	}

	closeFile(file);

	if (readSize != fileSize)
	{
		if (logger)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to read shader data file."
				"(path: %s)", path);
		}
		return false;
	}

	*code = shaderCode;
	*size = fileSize;
	return true;
}
ShaderData createShaderDataFromFile(
	const char* path,
	Logger logger)
{
	assert(path);

	ShaderData shaderData = calloc(1,
		sizeof(ShaderData_T));

	if (!shaderData)
		return NULL;

	uint8_t* code;
	size_t size;

	bool result = getShaderDataFromFile(
		path,
		logger,
		&code,
		&size);

	if (!result)
	{
		destroyShaderData(shaderData);
		return NULL;
	}

	shaderData->code = code;
	shaderData->size = size;
	return shaderData;
}
ShaderData createShaderDataFromPack(
	const char* path,
	PackReader packReader,
	Logger logger)
{
	assert(path);
	assert(packReader);

	ShaderData shaderData = calloc(1,
		sizeof(ShaderData_T));

	if (!shaderData)
		return NULL;

	const uint8_t* data;
	uint32_t size;

	PackResult packResult = readPackPathItemData(
		packReader,
		path,
		&data,
		&size);

	if (packResult != SUCCESS_PACK_RESULT)
	{
		if (logger)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to read pack shader data. "
				"(error: %s, path: %s)",
				packResultToString(packResult), path);
		}
		return NULL;
	}

	uint8_t* code;
	GraphicsAPI api = getGraphicsAPI();

	if (api == VULKAN_GRAPHICS_API)
	{
		code = malloc(size * sizeof(uint8_t));

		if (!code)
		{
			destroyShaderData(shaderData);
			return NULL;
		}

		memcpy(code, data, size * sizeof(uint8_t));
	}
	else if (api == OPENGL_GRAPHICS_API)
	{
		code = malloc((size + 1) * sizeof(uint8_t));

		if (!code)
		{
			destroyShaderData(shaderData);
			return NULL;
		}

		memcpy(code, data, size * sizeof(uint8_t));
		code[size] = '\0';
	}
	else
	{
		abort();
	}

	shaderData->code = code;
	shaderData->size = size;
	return shaderData;
}
void destroyShaderData(ShaderData shaderData)
{
	if (!shaderData)
		return;

	free(shaderData->code);
	free(shaderData);
}

const uint8_t* getShaderDataCode(ShaderData shaderData)
{
	assert(shaderData);
	return shaderData->code;
}
size_t getShaderDataSize(ShaderData shaderData)
{
	assert(shaderData);
	return shaderData->size;
}

Shader createShaderFromFile(
	const char* path,
	ShaderType type,
	Window window,
	Logger logger)
{
	assert(path);
	assert(type < SHADER_TYPE_COUNT);
	assert(window);

	uint8_t* code;
	size_t size;

	bool result = getShaderDataFromFile(
		path,
		logger,
		&code,
		&size);

	if (!result)
		return NULL;

	Shader shader;

	MpgxResult mpgxResult = createShader(
		window,
		type,
		code,
		size,
		&shader);

	free(code);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		if (logger)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to create shader from file. "
				"(error: %s, path: %s)",
				mpgxResultToString(mpgxResult), path);
		}
		return NULL;
	}

	return shader;
}
Shader createShaderFromPack(
	const char* path,
	ShaderType type,
	PackReader packReader,
	Window window,
	Logger logger)
{
	assert(path);
	assert(type < SHADER_TYPE_COUNT);
	assert(packReader);
	assert(window);

	const uint8_t* data;
	uint32_t size;

	PackResult packResult = readPackPathItemData(
		packReader,
		path,
		&data,
		&size);

	if (packResult != SUCCESS_PACK_RESULT)
	{
		if (logger)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to read pack shader data. "
				"(error: %s, path: %s)",
				packResultToString(packResult), path);
		}
		return NULL;
	}

	Shader shader;

	MpgxResult mpgxResult = createShader(
		window,
		type,
		data,
		size,
		&shader);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		if (logger)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to create shader from pack. "
				"(error: %s, path: %s)",
				mpgxResultToString(mpgxResult), path);
		}
		return NULL;
	}

	return shader;
}
