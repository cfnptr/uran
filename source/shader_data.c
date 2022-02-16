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

struct ShaderData_T
{
	uint8_t* code;
	size_t size;
};

ShaderData createShaderDataFromFile(
	const char* filePath,
	Logger logger)
{
	assert(filePath);

	ShaderData shaderData = calloc(1,
		sizeof(ShaderData_T));

	if (!shaderData)
		return NULL;

	FILE* file = openFile(filePath, "rb");

	if (!file)
	{
		if (logger)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to open shader data file.");
		}
		destroyShaderData(shaderData);
		return NULL;
	}

	int seekResult = seekFile(file, 0, SEEK_END);

	if (seekResult != 0)
	{
		if (logger)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to seek shader data file.");
		}
		closeFile(file);
		destroyShaderData(shaderData);
		return NULL;
	}

	size_t fileSize = tellFile(file);

	if (fileSize == 0)
	{
		if (logger)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to tell shader data file.");
		}
		closeFile(file);
		destroyShaderData(shaderData);
		return NULL;
	}

	shaderData->size = fileSize;
	seekResult = seekFile(file, 0, SEEK_SET);

	if (seekResult != 0)
	{
		if (logger)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to seek shader data file.");
		}
		closeFile(file);
		destroyShaderData(shaderData);
		return NULL;
	}

	uint8_t* code;
	size_t readSize;

	GraphicsAPI api = getGraphicsAPI();

	if (api == VULKAN_GRAPHICS_API)
	{
		code = malloc(fileSize * sizeof(uint8_t));

		if (!code)
		{
			closeFile(file);
			destroyShaderData(shaderData);
			return NULL;
		}

		readSize = fread(
			code,
			sizeof(uint8_t),
			fileSize,
			file);
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		code = malloc((fileSize + 1) * sizeof(uint8_t));

		if (!code)
		{
			closeFile(file);
			destroyShaderData(shaderData);
			return NULL;
		}

		readSize = fread(
			code,
			sizeof(uint8_t),
			fileSize,
			file);
		code[fileSize] = '\0';
	}
	else
	{
		abort();
	}

	closeFile(file);
	shaderData->code = code;

	if (readSize != fileSize)
	{
		if (logger)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to read shader data file.");
		}
		destroyShaderData(shaderData);
		return NULL;
	}

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

// TODO:
