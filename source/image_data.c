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

#include "uran/image_data.h"
#include "mpio/file.h"
#include "webp/decode.h"

struct ImageData_T
{
	uint8_t* pixels;
	Vec2I size;
	ImageFormat format;
};

inline static bool getImageDataFromData(
	const void* data,
	size_t size,
	ImageFormat format,
	Logger logger,
	uint8_t** _pixels,
	Vec2I* _imageSize)
{
	assert(data);
	assert(size);
	assert(format < IMAGE_FORMAT_COUNT);
	assert(_pixels);
	assert(_imageSize);

	uint8_t* pixels;
	Vec2I imageSize;

	if (format == R8G8B8A8_SRGB_IMAGE_FORMAT)
	{
		pixels = WebPDecodeRGBA(
			(const uint8_t*)data,
			size,
			&imageSize.x,
			&imageSize.y);
	}
	else
	{
#ifndef NDEBUG
		if (logger)
		{
			logMessage(logger, DEBUG_LOG_LEVEL,
				"Image data format is not supported.");
		}
#endif
		return false;
	}

	if (!pixels)
	{
		if (logger)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to decode WebP image data.");
		}
		return false;
	}

	*_pixels = pixels;
	*_imageSize = imageSize;
	return true;
}
ImageData createImageData(
	const void* data,
	size_t size,
	ImageFormat format,
	Logger logger)
{
	assert(data);
	assert(size > 0);
	assert(format < IMAGE_FORMAT_COUNT);

	ImageData imageData = calloc(1,
		sizeof(ImageData_T));

	if (!imageData)
		return NULL;

	imageData->format = format;

	uint8_t* pixels;
	Vec2I imageSize;

	bool result = getImageDataFromData(
		data,
		size,
		format,
		logger,
		&pixels,
		&imageSize);

	if (!result)
	{
		destroyImageData(imageData);
		return NULL;
	}

	imageData->pixels = pixels;
	imageData->size = imageSize;
	return imageData;
}
inline static bool getImageDataFromFile(
	const char* path,
	ImageFormat format,
	Logger logger,
	uint8_t** pixels,
	Vec2I* imageSize)
{
	assert(path);
	assert(format < IMAGE_FORMAT_COUNT);
	assert(pixels);
	assert(imageSize);

	FILE* file = openFile(path, "rb");

	if (!file)
	{
		if (logger)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to open WebP image data file."
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
				"Failed to seek WebP image data file."
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
				"Failed to tell WebP image data file."
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
				"Failed to seek WebP image data file."
				"(path: %s)", path);
		}
		closeFile(file);
		return false;
	}

	uint8_t* data = malloc(fileSize * sizeof(uint8_t));

	if (!data)
	{
		closeFile(file);
		return false;
	}

	size_t readSize = fread(
		data,
		sizeof(uint8_t),
		fileSize,
		file);

	closeFile(file);

	if (readSize != fileSize)
	{
		if (logger)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to read WebP image data file. "
				"(path: %s)", path);
		}
		free(data);
		return false;
	}

	bool result = getImageDataFromData(
		data,
		fileSize,
		format,
		logger,
		pixels,
		imageSize);

	free(data);
	return result;
}
ImageData createImageDataFromFile(
	const char* path,
	ImageFormat format,
	Logger logger)
{
	assert(path);
	assert(format < IMAGE_FORMAT_COUNT);

	ImageData imageData = calloc(1,
		sizeof(ImageData_T));

	if (!imageData)
		return NULL;

	imageData->format = format;

	uint8_t* pixels;
	Vec2I imageSize;

	bool result = getImageDataFromFile(
		path,
		format,
		logger,
		&pixels,
		&imageSize);

	if (!result)
	{
		destroyImageData(imageData);
		return NULL;
	}

	imageData->pixels = pixels;
	imageData->size = imageSize;
	return imageData;
}
inline static bool getImageDataFromPack(
	PackReader packReader,
	const char* path,
	ImageFormat format,
	Logger logger,
	uint8_t** pixels,
	Vec2I* imageSize)
{
	assert(packReader);
	assert(path);
	assert(format < IMAGE_FORMAT_COUNT);
	assert(pixels);
	assert(imageSize);

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
				"Failed to read pack WebP image data. "
				"(error: %s, path: %s)",
				packResultToString(packResult), path);
		}
		return false;
	}

	return getImageDataFromData(
		data,
		size,
		format,
		logger,
		pixels,
		imageSize);
}
ImageData createImageDataFromPack(
	const char* path,
	ImageFormat format,
	PackReader packReader,
	Logger logger)
{
	assert(path);
	assert(format < IMAGE_FORMAT_COUNT);
	assert(packReader);

	ImageData imageData = calloc(1,
		sizeof(ImageData_T));

	if (!imageData)
		return NULL;

	imageData->format = format;

	uint8_t* pixels;
	Vec2I imageSize;

	bool result = getImageDataFromPack(
		packReader,
		path,
		format,
		logger,
		&pixels,
		&imageSize);

	if (!result)
	{
		destroyImageData(imageData);
		return NULL;
	}

	imageData->pixels = pixels;
	imageData->size = imageSize;
	return imageData;
}
void destroyImageData(ImageData imageData)
{
	if (!imageData)
		return;

	WebPFree(imageData->pixels);
	free(imageData);
}

const uint8_t* getImageDataPixels(ImageData imageData)
{
	assert(imageData);
	return imageData->pixels;
}
Vec2I getImageDataSize(ImageData imageData)
{
	assert(imageData);
	return imageData->size;
}
ImageFormat getImageDataFormat(ImageData imageData)
{
	assert(imageData);
	return imageData->format;
}

Image createImageFromData(
	const void* data,
	size_t size,
	ImageType type,
	ImageFormat format,
	bool isConstant,
	Window window,
	Logger logger)
{
	assert(data);
	assert(size > 0);
	assert(type > 0);
	assert(format < IMAGE_FORMAT_COUNT);
	assert(window);

	uint8_t* pixels;
	Vec2I imageSize;

	bool result = getImageDataFromData(
		data,
		size,
		format,
		logger,
		&pixels,
		&imageSize);

	if (!result)
		return NULL;

	Image image;

	MpgxResult mpgxResult = createImage(
		window,
		type,
		IMAGE_2D,
		format,
		pixels,
		vec3I(imageSize.x, imageSize.y, 1),
		1,
		isConstant,
		&image);

	WebPFree(pixels);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		if (logger)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to create image from WebP data. (error: %s)",
				mpgxResultToString(mpgxResult));
		}
		return NULL;
	}

	return image;
}
Image createImageFromFile(
	const char* path,
	ImageType type,
	ImageFormat format,
	bool isConstant,
	Window window,
	Logger logger)
{
	assert(path);
	assert(type > 0);
	assert(format < IMAGE_FORMAT_COUNT);
	assert(window);

	uint8_t* pixels;
	Vec2I imageSize;

	bool result = getImageDataFromFile(
		path,
		format,
		logger,
		&pixels,
		&imageSize);

	if (!result)
		return NULL;

	Image image;

	MpgxResult mpgxResult = createImage(
		window,
		type,
		IMAGE_2D,
		format,
		pixels,
		vec3I(imageSize.x, imageSize.y, 1),
		1,
		isConstant,
		&image);

	WebPFree(pixels);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		if (logger)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to create image from WebP file. "
				"(error: %s, path: %s)",
				mpgxResultToString(mpgxResult), path);
		}
		return NULL;
	}

	return image;
}
Image createImageFromPack(
	const char* path,
	ImageType type,
	ImageFormat format,
	bool isConstant,
	PackReader packReader,
	Window window,
	Logger logger)
{
	assert(path);
	assert(type > 0);
	assert(format < IMAGE_FORMAT_COUNT);
	assert(packReader);
	assert(window);

	uint8_t* pixels;
	Vec2I imageSize;

	bool result = getImageDataFromPack(
		packReader,
		path,
		format,
		logger,
		&pixels,
		&imageSize);

	if (!result)
		return NULL;

	Image image;

	MpgxResult mpgxResult = createImage(
		window,
		type,
		IMAGE_2D,
		format,
		pixels,
		vec3I(imageSize.x, imageSize.y, 1),
		1,
		isConstant,
		&image);

	WebPFree(pixels);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		if (logger)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to create image from pack WebP. "
				"(error: %s, path: %s)",
				mpgxResultToString(mpgxResult), path);
		}
		return NULL;
	}

	return image;
}

// TODO: add image array reading, share WebP, file and pack buffers between them
//  possibly detect largest image in pack and change buffer size accordingly.
//
//  Also do the same for the shader data.
