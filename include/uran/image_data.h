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
 * Image data structure.
 */
typedef struct ImageData_T ImageData_T;
/*
 * Image data instance.
 */
typedef ImageData_T* ImageData;

/*
 * Create a new image data instance from the WebP data.
 * Returns image data instance on success, otherwise NULL.
 *
 * data - WebP image data.
 * size - WebP image data size in bytes.
 * format - image data format.
 * logger - logger instance or NULL.
 */
ImageData createImageData(
	const void* data,
	size_t size,
	ImageFormat format,
	Logger logger);
/*
 * Create a new image data instance from the WebP file.
 * Returns image data instance on success, otherwise NULL.
 *
 * path - WebP image data file path string.
 * format - image data format.
 * logger - logger instance or NULL.
 */
ImageData createImageDataFromFile(
	const char* path,
	ImageFormat format,
	Logger logger);
/*
 * Create a new image data instance from the pack WebP data.
 * Returns image data instance on success, otherwise NULL.
 *
 * path - WebP image data item path string.
 * format - image data format.
 * packReader - pack reader instance.
 * logger - logger instance or NULL.
 */
ImageData createImageDataFromPack(
	const char* path,
	ImageFormat format,
	PackReader packReader,
	Logger logger);
/*
 * Destroys image data instance.
 * imageData - image data instance or NULL.
 */
void destroyImageData(ImageData imageData);

/*
 * Returns image data pixels.
 * imageData - image data instance.
 */
const uint8_t* getImageDataPixels(ImageData imageData);
/*
 * Returns image data size.
 * imageData - image data instance.
 */
Vec2I getImageDataSize(ImageData imageData);
/*
 * Returns image data format.
 * imageData - image data instance.
 */
ImageFormat getImageDataFormat(ImageData imageData);

/*
 * Create a new image instance from the WebP data.
 * Returns image instance on success, otherwise NULL.
 *
 * data - WebP image data.
 * size - WebP image data size in bytes.
 * type - image type.
 * format - image data format.
 * isConstant - is image constant.
 * window - window instance.
 * logger - logger instance or NULL.
 */
Image createImageFromData(
	const void* data,
	size_t size,
	ImageType type,
	ImageFormat format,
	bool isConstant,
	Window window,
	Logger logger);
/*
 * Create a new image instance from the WebP data file.
 * Returns image instance on success, otherwise NULL.
 *
 * path - WebP image file path string.
 * type - image type.
 * format - image data format.
 * isConstant - is image constant.
 * window - window instance.
 * logger - logger instance or NULL.
 */
Image createImageFromFile(
	const char* path,
	ImageType type,
	ImageFormat format,
	bool isConstant,
	Window window,
	Logger logger);
/*
 * Create a new image instance from the pack WebP data.
 * Returns image instance on success, otherwise NULL.
 *
 * path - WebP image item path string.
 * type - image type.
 * format - image data format.
 * isConstant - is image constant.
 * packReader - pack reader instance.
 * window - window instance.
 * logger - logger instance or NULL.
 */
Image createImageFromPack(
	const char* path,
	ImageType type,
	ImageFormat format,
	bool isConstant,
	PackReader packReader,
	Window window,
	Logger logger);
