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

#include "uran/text.h"
#include "mpgx/_source/window.h"
#include "mpgx/_source/image.h"
#include "mpgx/_source/sampler.h"
#include "mpgx/_source/graphics_mesh.h"
#include "mpgx/_source/graphics_pipeline.h"


#define FT_CONFIG_OPTION_ERROR_STRINGS
#include "ft2build.h"
#include FT_FREETYPE_H

#include "cmmt/common.h"
#include <assert.h>

struct Font_T
{
	uint8_t* data;
	FT_Face face;
};

typedef struct Glyph
{
	uint32_t value;
	float positionX;
	float positionY;
	float positionZ;
	float positionW;
	float texCoordsX;
	float texCoordsY;
	float texCoordsZ;
	float texCoordsW;
	float advance;
	bool isVisible;
	uint8_t _alignment[3];
} Glyph;
struct FontAtlas_T
{
	GraphicsPipeline pipeline;
	Font* regularFonts;
	Font* boldFonts;
	Font* italicFonts;
	Font* boldItalicFonts;
	size_t fontCount;
	Glyph* regularGlyphs;
	Glyph* boldGlyphs;
	Glyph* italicGlyphs;
	Glyph* boldItalicGlyphs;
	size_t glyphCount;
	Image atlasImage;
	uint32_t fontSize;
	float newLineAdvance;
#if MPGX_SUPPORT_VULKAN
	uint8_t _alignment[3];
	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSet;
#endif
#ifndef NDEBUG
	bool isGenerated; // TODO: do not allow to use generated atlas
#endif
};

typedef struct TextVertex
{
	Vec2F position;
	Vec3F texCoords;
	SrgbColor color;
} TextVertex;

typedef struct BaseText
{
	FontAtlas fontAtlas;
	uint32_t* string;
	size_t capacity;
	size_t length;
	Vec2F size;
	SrgbColor color;
	AlignmentType alignment;
	bool isBold;
	bool isItalic;
	bool useTags;
	bool isConstant;
} BaseText;
#if MPGX_SUPPORT_VULKAN
typedef struct VkText
{
	FontAtlas fontAtlas;
	uint32_t* string;
	size_t capacity;
	size_t length;
	Vec2F size;
	SrgbColor color;
	AlignmentType alignment;
	bool isBold;
	bool isItalic;
	bool useTags;
	bool isConstant;
	uint8_t _alignment[3];
	uint32_t indexCount;
	Buffer vertexBuffer;
} VkText;
#endif
#if MPGX_SUPPORT_OPENGL
typedef struct GlText
{
	FontAtlas fontAtlas;
	uint32_t* string;
	size_t capacity;
	size_t length;
	Vec2F size;
	SrgbColor color;
	AlignmentType alignment;
	bool isBold;
	bool isItalic;
	bool useTags;
	bool isConstant;
	uint8_t _alignment[3];
	GraphicsMesh mesh;
} GlText;
#endif

union Text_T
{
	BaseText base;
#if MPGX_SUPPORT_VULKAN
	VkText vk;
#endif
#if MPGX_SUPPORT_OPENGL
	GlText gl;
#endif
};

typedef struct VertexPushConstants
{
	Mat4F mvp;
} VertexPushConstants;
typedef struct FragmentPushConstants
{
	LinearColor color;
} FragmentPushConstants;
typedef struct BaseHandle
{
	Sampler sampler;
	VertexPushConstants vpc;
	FragmentPushConstants fpc;
	Text* texts;
	size_t textCapacity;
	size_t textCount;
	TextVertex* vertexBuffer;
	size_t vertexCapacity;
	Buffer indexBuffer;
} BaseHandle;
#if MPGX_SUPPORT_VULKAN
typedef struct VkHandle
{
	Sampler sampler;
	VertexPushConstants vpc;
	FragmentPushConstants fpc;
	Text* texts;
	size_t textCapacity;
	size_t textCount;
	TextVertex* vertexBuffer;
	size_t vertexCapacity;
	Buffer indexBuffer;
	VkDescriptorSetLayout descriptorSetLayout;
} VkHandle;
#endif
#if MPGX_SUPPORT_OPENGL
typedef struct GlHandle
{
	Sampler sampler;
	VertexPushConstants vpc;
	FragmentPushConstants fpc;
	Text* texts;
	size_t textCapacity;
	size_t textCount;
	TextVertex* vertexBuffer;
	size_t vertexCapacity;
	Buffer indexBuffer;
	GLint mvpLocation;
	GLint atlasLocation;
	GLint colorLocation;
} GlHandle;
#endif
typedef union Handle_T
{
	BaseHandle base;
#if MPGX_SUPPORT_VULKAN
	VkHandle vk;
#endif
#if MPGX_SUPPORT_OPENGL
	GlHandle gl;
#endif
} Handle_T;

typedef Handle_T* Handle;

static bool textInitialized = false;
static FT_Library ftLibrary = NULL;

bool initializeText(Logger logger)
{
	if (textInitialized)
		return false;

	FT_Error ftResult = FT_Init_FreeType(&ftLibrary);

	if (ftResult != 0)
	{
		if (logger)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to initialize FreeType. (error: %s)",
				FT_Error_String(ftResult));
		}
		return false;
	}

	textInitialized = true;
	return true;
}
void terminateText(Logger logger)
{
	if (!textInitialized)
		return;

	FT_Error ftResult = FT_Done_FreeType(ftLibrary);

	if (ftResult != 0)
	{
		if (logger)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to terminate FreeType. (error: %s)",
				FT_Error_String(ftResult));
		}
		abort();
	}

	ftLibrary = NULL;
}
bool isTextInitialized()
{
	return textInitialized;
}

size_t stringUTF8toUTF32(
	const char* source,
	size_t sourceLength,
	uint32_t* destination)
{
	assert(source);
	assert(sourceLength > 0);
	assert(destination);

	size_t i = 0, length = 0;

	while(i < sourceLength)
	{
		char value = source[i];

		if ((value & 0b10000000) == 0)
		{
			destination[length] = (uint32_t)source[i];
			i += 1;
		}
		else if (i + 1 < sourceLength &&
			!((value & 0b11100000) ^ 0b11000000 |
			(source[i + 1] & 0b11000000) ^ 0b10000000))
		{
			destination[length] =
				(uint32_t)(source[i] & 0b00011111) << 6 |
				(uint32_t)(source[i + 1] & 0b00111111);
			i += 2;
		}
		else if (i + 2 < sourceLength &&
			!((value & 0b11110000) ^ 0b11100000 |
			(source[i + 1] & 0b11000000) ^ 0b10000000 |
			(source[i + 2] & 0b11000000) ^ 0b10000000))
		{
			destination[length] =
				(uint32_t)(source[i] & 0b00001111) << 12 |
				(uint32_t)(source[i + 1] & 0b00111111) << 6 |
				(uint32_t)(source[i + 2] & 0b00111111);
			i += 3;
		}
		else if (i + 3 < sourceLength &&
			!((value & 0b11111000) ^ 0b11110000 |
			(source[i + 1] & 0b11000000) ^ 0b10000000 |
			(source[i + 2] & 0b11000000) ^ 0b10000000 |
			(source[i + 3] & 0b11000000) ^ 0b10000000))
		{
			destination[length] =
				(uint32_t)(source[i] & 0b00000111) << 18 |
				(uint32_t)(source[i + 1] & 0b00111111) << 12 |
				(uint32_t)(source[i + 2] & 0b00111111) << 6 |
				(uint32_t)(source[i + 3] & 0b00111111);
			i += 4;
		}
		else
		{
			return 0;
		}

		length++;
	}

	return length;
}

MpgxResult allocateStringUTF8(
	const uint32_t* source,
	size_t sourceLength,
	char** destination,
	size_t* destinationLength)
{
	assert(source);
	assert(sourceLength > 0);
	assert(destination);
	assert(destinationLength);

	size_t length = 0;

	for (size_t i = 0; i < sourceLength; i++)
	{
		uint32_t value = source[i];

		if (value < 128) length += 1;
		else if (value < 2048) length += 2;
		else if (value < 65536) length += 3;
		else if (value < 2097152) length += 4;
		else return BAD_VALUE_MPGX_RESULT;
	}

	char* destinationArray = malloc(
		(length + 1) * sizeof(char));

	if (!destinationArray)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	for (size_t i = 0, j = 0; i < sourceLength; i++)
	{
		uint32_t value = source[i];

		if (value < 128)
		{
			destinationArray[j] = (char)value;
			j += 1;
		}
		else if (value < 2048)
		{
			destinationArray[j] = (char)(((value >> 6) | 0b11000000) & 0b11011111);
			destinationArray[j + 1] = (char)((value | 0b10000000) & 0b10111111);
			j += 2;
		}
		else if (value < 65536)
		{
			destinationArray[j] = (char)(((value >> 12) | 0b11100000) & 0b11101111);
			destinationArray[j + 1] = (char)(((value >> 6) | 0b10000000) & 0b10111111);
			destinationArray[j + 2] = (char)((value | 0b10000000) & 0b10111111);
			j += 3;
		}
		else
		{
			destinationArray[j] = (char)(((value >> 18) | 0b11110000) & 0b11110111);
			destinationArray[j + 1] = (char)(((value >> 12) | 0b10000000) & 0b10111111);
			destinationArray[j + 2] = (char)(((value >> 6) | 0b10000000) & 0b10111111);
			destinationArray[j + 3] = (char)((value | 0b10000000) & 0b10111111);
			j += 4;
		}
	}

	destinationArray[length] = '\0';

	*destination = destinationArray;
	*destinationLength = length;
	return SUCCESS_MPGX_RESULT;
}
bool validateStringUTF8(
	const char* string,
	size_t stringLength)
{
	assert(string);
	assert(stringLength > 0);

	for (size_t i = 0; i < stringLength; i++)
	{
		char value = string[i];

		if ((value & 0b10000000) ^ 0 ||

			(i + 1 >= stringLength ||
			(value & 0b11100000) ^ 0b11000000 |
			(string[i + 1] & 0b11000000) ^ 0b10000000) ||

			(i + 2 >= stringLength ||
			(value & 0b11110000) ^ 0b11100000 |
			(string[i + 1] & 0b11000000) ^ 0b10000000 |
			(string[i + 2] & 0b11000000) ^ 0b10000000) ||

			(i + 3 >= stringLength ||
			(value & 0b11111000) ^ 0b11110000 |
			(string[i + 1] & 0b11000000) ^ 0b10000000 |
			(string[i + 2] & 0b11000000) ^ 0b10000000 |
			(string[i + 3] & 0b11000000) ^ 0b10000000))
		{
			return false;
		}
	}

	return true;
}

MpgxResult allocateStringUTF32(
	const char* source,
	size_t sourceLength,
	uint32_t** destination,
	size_t* destinationLength)
{
	assert(source);
	assert(sourceLength > 0);
	assert(destination);
	assert(destinationLength);

	size_t length = 0;

	for (size_t i = 0; i < sourceLength;)
	{
		char value = source[i];

		if ((value & 0b10000000) == 0)
		{
			i += 1;
		}
		else if (i + 1 < sourceLength &&
			!((value & 0b11100000) ^ 0b11000000 |
			(source[i + 1] & 0b11000000) ^ 0b10000000))
		{
			i += 2;
		}
		else if (i + 2 < sourceLength &&
			!((value & 0b11110000) ^ 0b11100000 |
			(source[i + 1] & 0b11000000) ^ 0b10000000 |
			(source[i + 2] & 0b11000000) ^ 0b10000000))
		{
			i += 3;
		}
		else if (i + 3 < sourceLength &&
			!((value & 0b11111000) ^ 0b11110000 |
			(source[i + 1] & 0b11000000) ^ 0b10000000 |
			(source[i + 2] & 0b11000000) ^ 0b10000000 |
			(source[i + 3] & 0b11000000) ^ 0b10000000))
		{
			i += 4;
		}
		else
		{
			return BAD_VALUE_MPGX_RESULT;
		}

		length++;
	}

	uint32_t* destinationArray = malloc(
		(length + 1) * sizeof(uint32_t));

	if (!destinationArray)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	for (size_t i = 0, j = 0; i < sourceLength; j++)
	{
		char value = source[i];

		if ((value & 0b10000000) == 0)
		{
			destinationArray[j] = (uint32_t)source[i];
			i += 1;
		}
		else if ((value & 0b11100000) == 0b11000000)
		{
			destinationArray[j] =
				(uint32_t)(source[i] & 0b00011111) << 6 |
				(uint32_t)(source[i + 1] & 0b00111111);
			i += 2;
		}
		else if ((value & 0b11110000) == 0b11100000)
		{
			destinationArray[j] =
				(uint32_t)(source[i] & 0b00001111) << 12 |
				(uint32_t)(source[i + 1] & 0b00111111) << 6 |
				(uint32_t)(source[i + 2] & 0b00111111);
			i += 3;
		}
		else
		{
			destinationArray[j] =
				(uint32_t)(source[i] & 0b00000111) << 18 |
				(uint32_t)(source[i + 1] & 0b00111111) << 12 |
				(uint32_t)(source[i + 2] & 0b00111111) << 6 |
				(uint32_t)(source[i + 3] & 0b00111111);
			i += 4;
		}
	}

	destinationArray[length] = 0;

	*destination = destinationArray;
	*destinationLength = length;
	return SUCCESS_MPGX_RESULT;
}
bool validateStringUTF32(
	const uint32_t* string,
	size_t stringLength)
{
	assert(string);
	assert(stringLength > 0);

	for (size_t i = 0; i < stringLength; i++)
	{
		uint32_t value = string[i];

		if (value == 0 || value >= 2097152)
			return false;
	}

	return true;
}

Font createFont(
	const void* data,
	size_t size,
	size_t index,
	Logger logger)
{
	assert(data);
	assert(size > 0);

	if (!textInitialized)
		return NULL;

	Font font = calloc(1,
		sizeof(Font_T));

	if (!font)
		return NULL;

	uint8_t* dataArray = malloc(
		size * sizeof(uint8_t));

	if (!dataArray)
	{
		destroyFont(font);
		return NULL;
	}

	font->data = dataArray;
	memcpy(dataArray, data, size);

	FT_Face face;

	FT_Error ftResult = FT_New_Memory_Face(
		ftLibrary,
		dataArray,
		(FT_Long)size,
		(FT_Long)index,
		&face);

	if (ftResult != 0)
	{
		if (logger)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to create FreeType memory face. (error: %s)",
				FT_Error_String(ftResult));
		}
		destroyFont(font);
		return NULL;
	}

	font->face = face;

	ftResult = FT_Select_Charmap(face,
		FT_ENCODING_UNICODE);

	if (ftResult != 0)
	{
		if (logger)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to select FreeType char map. (error: %s)",
				FT_Error_String(ftResult));
		}
		destroyFont(font);
		return NULL;
	}

	return font;
}
Font createFontFromFile(
	const void* path,
	size_t index,
	Logger logger)
{
	assert(path);

	if (!textInitialized)
		return NULL;

	Font font = calloc(1,
		sizeof(Font_T));

	if (!font)
		return NULL;

	font->data = NULL;

	FT_Face face;

	FT_Error ftResult = FT_New_Face(
		ftLibrary,
		path,
		(FT_Long)index,
		&face);

	if (ftResult != 0)
	{
		if (logger)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to create FreeType face. "
				"(error: %s, path: %s)",
				FT_Error_String(ftResult), path);
		}
		destroyFont(font);
		return NULL;
	}

	font->face = face;

	ftResult = FT_Select_Charmap(face,
		FT_ENCODING_UNICODE);

	if (ftResult != 0)
	{
		if (logger)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to select FreeType char map. "
				"(error: %s, path: %s)",
				FT_Error_String(ftResult), path);
		}
		destroyFont(font);
		return NULL;
	}

	return font;
}
Font createFontFromPack(
	PackReader packReader,
	const char* path,
	size_t index,
	Logger logger)
{
	assert(packReader);
	assert(path);

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
				"Failed to read pack font data. "
				"(error: %s, path: %s)",
				packResultToString(packResult), path);
		}
		return NULL;
	}

	return createFont(
		data,
		size,
		index,
		logger);
}
void destroyFont(Font font)
{
	if (!font)
		return;

	assert(textInitialized);

	if (font->face)
		FT_Done_Face(font->face);
	free(font->data);
	free(font);
}

static int compareGlyph(const void* a, const void* b)
{
	// NOTE: a and b should not be NULL!
	// Skipping assertion for debug build speed.

	if (((Glyph*)a)->value < ((Glyph*)b)->value)
		return -1;
	if (((Glyph*)a)->value > ((Glyph*)b)->value)
		return 1;
	return 0;
}
inline static MpgxResult createGlyphs(
	const uint32_t* string,
	size_t stringLength,
	Glyph** glyphs,
	size_t* glyphCount)
{
	assert(string);
	assert(stringLength > 0);
	assert(glyphs);
	assert(glyphCount > 0);

	Glyph* glyphArray = malloc(
		stringLength * sizeof(Glyph));

	if (!glyphArray)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	size_t count = 0;

	for (size_t i = 0; i < stringLength; i++)
	{
		uint32_t value = string[i];

		if (value == '\n') continue;
		else if (value == '\t') value = ' ';

		Glyph* glyph = bsearch(
			&value,
			glyphArray,
			count,
			sizeof(Glyph),
			compareGlyph);

		if (!glyph)
		{
			glyphArray[count++].value = value;

			qsort(glyphArray,
				count,
				sizeof(Glyph),
				compareGlyph);
		}
	}

	if (count == 0)
	{
		free(glyphArray);
		return BAD_VALUE_MPGX_RESULT;
	}

	*glyphs = glyphArray;
	*glyphCount = count;
	return SUCCESS_MPGX_RESULT;
}

inline static bool setFtPixelSize(
	FT_Face face,
	uint32_t size,
	Logger logger)
{
	assert(face);
	assert(size > 0);

	FT_Error ftResult = FT_Set_Pixel_Sizes(
		face,
		0,
		(FT_UInt)size);

	if (ftResult != 0)
	{
		if (logger)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to set FreeType pixel sizes. (error: %s)",
				FT_Error_String(ftResult));
		}
		return false;
	}

	return true;
}
inline static bool fillAtlasPixels(
	Font* fonts,
	size_t fontCount,
	uint32_t fontSize,
	Glyph* glyphs,
	size_t glyphCount,
	uint32_t glyphLength,
	uint32_t pixelLength,
	uint8_t fontIndex,
	uint8_t* pixels,
	Logger logger)
{
	assert(fonts);
	assert(fontCount > 0);
	assert(fontSize > 0);
	assert(glyphs);
	assert(glyphCount > 0);
	assert(glyphLength > 0);
	assert(pixels);

	for (size_t i = 0; i < fontCount; i++)
	{
		bool result = setFtPixelSize(
			fonts[i]->face,
			fontSize,
			logger);

		if (!result)
			return false;
	}

	FT_Face mainFace = fonts[0]->face;

	for (size_t i = 0; i < glyphCount; i++)
	{
		Glyph glyph;
		glyph.value = glyphs[i].value;

		FT_UInt charIndex = FT_Get_Char_Index(
			mainFace,
			glyph.value);
		FT_Face charFace = mainFace;

		if (charIndex == 0)
		{
			for (size_t j = 1; j < fontCount; j++)
			{
				FT_Face face = fonts[j]->face;

				charIndex = FT_Get_Char_Index(
					face,
					glyph.value);

				if (charIndex != 0)
				{
					charFace = face;
					break;
				}
			}
		}

		FT_Error ftResult = FT_Load_Glyph(
			charFace,
			charIndex,
			FT_LOAD_RENDER);

		if (ftResult != 0)
		{
			if (logger)
			{
				logMessage(logger, ERROR_LOG_LEVEL,
					"Failed to load FreeType glyph. (error: %s)",
					FT_Error_String(ftResult));
			}
			return false;
		}

		FT_GlyphSlot glyphSlot = charFace->glyph;
		uint32_t glyphWidth = glyphSlot->bitmap.width;
		uint32_t glyphHeight = glyphSlot->bitmap.rows;

		glyph.advance = ((float)glyphSlot->advance.x /
			64.0f) / (float)fontSize;

		if (glyphWidth * glyphHeight == 0)
		{
			glyph.isVisible = false;
		}
		else
		{
			uint8_t* bitmap = glyphSlot->bitmap.buffer;
			uint32_t glyphPosY = (uint32_t)(i / glyphLength);
			uint32_t glyphPosX = (uint32_t)(i - glyphPosY * glyphLength);
			uint32_t pixelPosX = glyphPosX * fontSize;
			uint32_t pixelPosY = glyphPosY * fontSize;

			glyph.positionX = (float)glyphSlot->bitmap_left / (float)fontSize;
			glyph.positionY = ((float)glyphSlot->bitmap_top - (float)glyphHeight) / (float)fontSize;
			glyph.positionZ = glyph.positionX + (float)glyphWidth / (float)fontSize;
			glyph.positionW = glyph.positionY + (float)glyphHeight /(float)fontSize;
			glyph.texCoordsX = (float)pixelPosX / (float)pixelLength; //tpl
			glyph.texCoordsY = (float)pixelPosY / (float)pixelLength; //tpl
			glyph.texCoordsZ = glyph.texCoordsX + (float)glyphWidth / (float)pixelLength; //tpl
			glyph.texCoordsW = glyph.texCoordsY + (float)glyphHeight / (float)pixelLength; //tpl
			glyph.isVisible = true;

			for (uint32_t y = 0; y < glyphHeight; y++)
			{
				for (uint32_t x = 0; x < glyphWidth; x++)
				{
					pixels[fontIndex + ((x + pixelPosX) +
						(y + pixelPosY) * pixelLength) * 4] =
						bitmap[x + y * glyphWidth];
				}
			}
		}

		glyphs[i] = glyph;
	}

	return true;
}

#if MPGX_SUPPORT_VULKAN
inline static MpgxResult createVkDescriptorPool(
	VkDevice device,
	VkDescriptorPool* descriptorPool)
{
	assert(device);
	assert(descriptorPool);

	VkDescriptorPoolSize descriptorPoolSizes[1] = {
		{
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			1,
		},
	};
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		NULL,
		0,
		1,
		1,
		descriptorPoolSizes,
	};

	VkDescriptorPool descriptorPoolInstance;

	VkResult vkResult = vkCreateDescriptorPool(
		device,
		&descriptorPoolCreateInfo,
		NULL,
		&descriptorPoolInstance);

	if (vkResult != VK_SUCCESS)
		return vkToMpgxResult(vkResult);

	*descriptorPool = descriptorPoolInstance;
	return SUCCESS_MPGX_RESULT;
}
inline static MpgxResult createVkDescriptorSet(
	VkDevice device,
	VkDescriptorSetLayout descriptorSetLayout,
	VkDescriptorPool descriptorPool,
	VkSampler sampler,
	VkImageView imageView,
	VkDescriptorSet* descriptorSet)
{
	assert(device);
	assert(descriptorSetLayout);
	assert(descriptorPool);
	assert(sampler);
	assert(imageView);
	assert(descriptorSet);

	VkDescriptorSet descriptorSetInstance;

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		NULL,
		descriptorPool,
		1,
		&descriptorSetLayout,
	};

	VkResult vkResult = vkAllocateDescriptorSets(
		device,
		&descriptorSetAllocateInfo,
		&descriptorSetInstance);

	if (vkResult != VK_SUCCESS)
		return vkToMpgxResult(vkResult);

	VkDescriptorImageInfo descriptorImageInfos[1] = {
		{
			sampler,
			imageView,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		},
	};
	VkWriteDescriptorSet writeDescriptorSets[1] = {
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			NULL,
			descriptorSetInstance,
			0,
			0,
			1,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			descriptorImageInfos,
			NULL,
			NULL,
		},
	};

	vkUpdateDescriptorSets(
		device,
		1,
		writeDescriptorSets,
		0,
		NULL);

	*descriptorSet = descriptorSetInstance;
	return SUCCESS_MPGX_RESULT;
}
#endif

MpgxResult createFontAtlas(
	GraphicsPipeline textPipeline,
	Font* regularFonts,
	Font* boldFonts,
	Font* italicFonts,
	Font* boldItalicFonts,
	size_t fontCount,
	uint32_t fontSize,
	const uint32_t* chars,
	size_t charCount,
	Logger logger,
	FontAtlas* fontAtlas)
{
	assert(textPipeline);
	assert(regularFonts);
	assert(boldFonts);
	assert(italicFonts);
	assert(boldItalicFonts);
	assert(fontCount > 0);
	assert(fontSize > 0);
	assert(chars);
	assert(charCount > 0);
	assert(fontAtlas);
	assert(fontSize % 2 == 0);
	assert(textInitialized);

	FontAtlas fontAtlasInstance = calloc(
		1, sizeof(FontAtlas_T));

	if (!fontAtlasInstance)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	fontAtlasInstance->pipeline = textPipeline;
	fontAtlasInstance->fontSize = fontSize;

	FT_Face defaultFace = regularFonts[0]->face;

	bool result = setFtPixelSize(
		defaultFace,
		fontSize,
		logger);

	if (!result)
	{
		destroyFontAtlas(fontAtlasInstance);
		return UNKNOWN_ERROR_MPGX_RESULT;
	}

	fontAtlasInstance->newLineAdvance =
		((float)defaultFace->size->metrics.height /
		64.0f) / (float)fontSize;

	Font* regularFontArray = malloc(
		fontCount * sizeof(Font));

	if (!regularFontArray)
	{
		destroyFontAtlas(fontAtlasInstance);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	memcpy(regularFontArray, regularFonts,
		fontCount * sizeof(Font));
	fontAtlasInstance->regularFonts = regularFontArray;
	fontAtlasInstance->fontCount = fontCount;

	Font* boldFontArray = malloc(fontCount * sizeof(Font));

	if (!boldFontArray)
	{
		destroyFontAtlas(fontAtlasInstance);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	memcpy(boldFontArray, boldFonts,
		fontCount * sizeof(Font));
	fontAtlasInstance->boldFonts = boldFontArray;

	Font* italicFontArray = malloc(fontCount * sizeof(Font));

	if (!italicFontArray)
	{
		destroyFontAtlas(fontAtlasInstance);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	memcpy(italicFontArray, italicFonts,
		fontCount * sizeof(Font));
	fontAtlasInstance->italicFonts = italicFontArray;

	Font* boldItalicFontArray = malloc(fontCount * sizeof(Font));

	if (!boldItalicFontArray)
	{
		destroyFontAtlas(fontAtlasInstance);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	memcpy(boldItalicFontArray, boldItalicFonts,
		fontCount * sizeof(Font));
	fontAtlasInstance->boldItalicFonts = boldItalicFontArray;

	Glyph* regularGlyphs;
	size_t glyphCount;

	MpgxResult mpgxResult = createGlyphs(
		chars,
		charCount,
		&regularGlyphs,
		&glyphCount);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyFontAtlas(fontAtlasInstance);
		return mpgxResult;
	}

	fontAtlasInstance->regularGlyphs = regularGlyphs;
	fontAtlasInstance->glyphCount = glyphCount;

	Glyph* boldGlyphs = malloc(glyphCount * sizeof(Glyph));

	if (!boldGlyphs)
	{
		destroyFontAtlas(fontAtlasInstance);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	memcpy(boldGlyphs, regularGlyphs,
		glyphCount * sizeof(Glyph));
	fontAtlasInstance->boldGlyphs = boldGlyphs;

	Glyph* italicGlyphs = malloc(glyphCount * sizeof(Glyph));

	if (!italicGlyphs)
	{
		destroyFontAtlas(fontAtlasInstance);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	memcpy(italicGlyphs, regularGlyphs,
		glyphCount * sizeof(Glyph));
	fontAtlasInstance->italicGlyphs = italicGlyphs;

	Glyph* boldItalicGlyphs = malloc(glyphCount * sizeof(Glyph));

	if (!boldItalicGlyphs)
	{
		destroyFontAtlas(fontAtlasInstance);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	memcpy(boldItalicGlyphs, regularGlyphs,
		glyphCount * sizeof(Glyph));
	fontAtlasInstance->boldItalicGlyphs = boldItalicGlyphs;

	uint32_t glyphLength = (uint32_t)ceil(sqrt((double)glyphCount));
	uint32_t pixelLength = glyphLength * fontSize;

	uint8_t* pixels = calloc(
		pixelLength * pixelLength,
		4 * sizeof(uint8_t));

	if (!pixels)
	{
		destroyFontAtlas(fontAtlasInstance);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	result = fillAtlasPixels(
		regularFonts,
		fontCount,
		fontSize,
		regularGlyphs,
		glyphCount,
		glyphLength,
		pixelLength,
		0,
		pixels,
		logger);

	if (!result)
	{
		free(pixels);
		destroyFontAtlas(fontAtlasInstance);
		return UNKNOWN_ERROR_MPGX_RESULT;
	}

	result = fillAtlasPixels(
		boldFonts,
		fontCount,
		fontSize,
		boldGlyphs,
		glyphCount,
		glyphLength,
		pixelLength,
		1,
		pixels,
		logger);

	if (!result)
	{
		free(pixels);
		destroyFontAtlas(fontAtlasInstance);
		return UNKNOWN_ERROR_MPGX_RESULT;
	}

	result = fillAtlasPixels(
		italicFonts,
		fontCount,
		fontSize,
		italicGlyphs,
		glyphCount,
		glyphLength,
		pixelLength,
		2,
		pixels,
		logger);

	if (!result)
	{
		free(pixels);
		destroyFontAtlas(fontAtlasInstance);
		return mpgxResult;
	}

	result = fillAtlasPixels(
		boldItalicFonts,
		fontCount,
		fontSize,
		boldItalicGlyphs,
		glyphCount,
		glyphLength,
		pixelLength,
		3,
		pixels,
		logger);

	if (!result)
	{
		free(pixels);
		destroyFontAtlas(fontAtlasInstance);
		return mpgxResult;
	}

	Window window = textPipeline->base.window;

	Image atlasImage;

	mpgxResult = createImage(
		window,
		SAMPLED_IMAGE_TYPE,
		IMAGE_2D,
		R8G8B8A8_UNORM_IMAGE_FORMAT,
		pixels,
		vec3I(
			(cmmt_int_t)pixelLength,
			(cmmt_int_t)pixelLength,
			1),
		1,
		true,
		&atlasImage);

	free(pixels);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyFontAtlas(fontAtlasInstance);
		return mpgxResult;
	}

	fontAtlasInstance->atlasImage = atlasImage;

#if MPGX_SUPPORT_VULKAN
	GraphicsAPI api = getGraphicsAPI();

	if (api == VULKAN_GRAPHICS_API)
	{
		VkWindow vkWindow = getVkWindow(window);
		Handle pipelineHandle = textPipeline->vk.handle;

		VkDescriptorPool descriptorPool;

		mpgxResult = createVkDescriptorPool(
			vkWindow->device,
			&descriptorPool);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			destroyFontAtlas(fontAtlasInstance);
			return mpgxResult;
		}

		fontAtlasInstance->descriptorPool = descriptorPool;

		VkDescriptorSet descriptorSet;

		mpgxResult = createVkDescriptorSet(
			vkWindow->device,
			pipelineHandle->vk.descriptorSetLayout,
			descriptorPool,
			pipelineHandle->vk.sampler->vk.handle,
			atlasImage->vk.imageView,
			&descriptorSet);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			destroyFontAtlas(fontAtlasInstance);
			return mpgxResult;
		}

		fontAtlasInstance->descriptorSet = descriptorSet;
	}
	else
	{
		fontAtlasInstance->descriptorPool = NULL;
		fontAtlasInstance->descriptorSet = NULL;
	}
#endif

	*fontAtlas = fontAtlasInstance;
	return SUCCESS_MPGX_RESULT;
}
MpgxResult createAsciiFontAtlas(
	GraphicsPipeline textPipeline,
	Font* regularFonts,
	Font* boldFonts,
	Font* italicFonts,
	Font* boldItalicFonts,
	size_t fontCount,
	uint32_t fontSize,
	Logger logger,
	FontAtlas* fontAtlas)
{
	assert(textPipeline);
	assert(regularFonts);
	assert(boldFonts);
	assert(italicFonts);
	assert(boldItalicFonts);
	assert(fontCount > 0);
	assert(fontSize > 0);
	assert(fontAtlas);
	assert(fontSize % 2 == 0);
	assert(textInitialized);

	return createFontAtlas(
		textPipeline,
		regularFonts,
		boldFonts,
		italicFonts,
		boldItalicFonts,
		fontCount,
		fontSize,
		printableAscii32,
		sizeof(printableAscii32) / sizeof(uint32_t),
		logger,
		fontAtlas);
}
void destroyFontAtlas(FontAtlas fontAtlas)
{
	if (!fontAtlas)
		return;

	assert(textInitialized);

#if MPGX_SUPPORT_VULKAN
	GraphicsAPI api = getGraphicsAPI();

	if (api == VULKAN_GRAPHICS_API)
	{
		VkWindow vkWindow = getVkWindow(
			fontAtlas->pipeline->base.window);

		vkDestroyDescriptorPool(
			vkWindow->device,
			fontAtlas->descriptorPool,
			NULL);
	}
#endif

	destroyImage(fontAtlas->atlasImage);
	free(fontAtlas->boldItalicGlyphs);
	free(fontAtlas->italicGlyphs);
	free(fontAtlas->boldGlyphs);
	free(fontAtlas->regularGlyphs);
	free(fontAtlas->boldItalicFonts);
	free(fontAtlas->italicFonts);
	free(fontAtlas->boldFonts);
	free(fontAtlas->regularFonts);
	free(fontAtlas);
}

GraphicsPipeline getFontAtlasPipeline(FontAtlas fontAtlas)
{
	assert(fontAtlas);
	assert(textInitialized);
	return fontAtlas->pipeline;
}
Font* getFontAtlasRegularFonts(FontAtlas fontAtlas)
{
	assert(fontAtlas);
	assert(textInitialized);
	return fontAtlas->regularFonts;
}
Font* getFontAtlasBoldFonts(FontAtlas fontAtlas)
{
	assert(fontAtlas);
	assert(textInitialized);
	return fontAtlas->boldFonts;
}
Font* getFontAtlasItalicFonts(FontAtlas fontAtlas)
{
	assert(fontAtlas);
	assert(textInitialized);
	return fontAtlas->italicFonts;
}
Font* getFontAtlasBoldItalicFonts(FontAtlas fontAtlas)
{
	assert(fontAtlas);
	assert(textInitialized);
	return fontAtlas->boldItalicFonts;
}
size_t getFontAtlasFontCount(FontAtlas fontAtlas)
{
	assert(fontAtlas);
	assert(textInitialized);
	return fontAtlas->fontCount;
}
uint32_t getFontAtlasFontSize(FontAtlas fontAtlas)
{
	assert(fontAtlas);
	assert(textInitialized);
	return fontAtlas->fontSize;
}

inline static bool hexToColor(
	const uint32_t* string,
	uint8_t* _value)
{
	// Note: skipping assertions for debug build speed.

	uint8_t value = 0;
	uint32_t charValue = string[0];

	if (charValue > '/' && charValue < ':')
		value = (charValue - '/') << 4u;
	else if (charValue > '`' && charValue < 'g')
		value = (charValue - 'W') << 4u;
	else
		return false;

	charValue = string[1];

	if (charValue > '/' && charValue < ':')
		value |= charValue - '/';
	else if (charValue > '`' && charValue < 'g')
		value |= charValue - 'W';
	else
		return false;

	*_value = value;
	return true;
}
inline static bool fillVertices(
	const uint32_t* string,
	size_t length,
	const Glyph* regularGlyphs,
	const Glyph* boldGlyphs,
	const Glyph* italicGlyphs,
	const Glyph* boldItalicGlyphs,
	size_t glyphCount,
	float newLineAdvance,
	AlignmentType alignment,
	SrgbColor color,
	bool isBold,
	bool isItalic,
	bool useTags,
	TextVertex* vertices,
	uint32_t* vertexCount,
	Vec2F* textSize)
{
	assert(regularGlyphs);
	assert(boldGlyphs);
	assert(italicGlyphs);
	assert(boldItalicGlyphs);
	assert(glyphCount > 0);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(vertices);
	assert(vertexCount);
	assert(textSize);

	assert(length == 0 ||
		(length > 0 && string));

	SrgbColor useColor = color;
	bool useBold = isBold, useItalic = isItalic;
	float sizeX = 0.0f, sizeY = 0.0f;
	float vertexOffsetX = 0.0f, vertexOffsetY = -newLineAdvance * 0.5f;
	uint32_t vertexIndex = 0, lastNewLineIndex = 0;

	const Glyph* glyphs;
	float atlasIndex;

	if (isBold & isItalic)
	{
		glyphs = boldItalicGlyphs;
		atlasIndex = 3.0f;
	}
	else if (isItalic)
	{
		glyphs = italicGlyphs;
		atlasIndex = 2.0f;
	}
	else if (isBold)
	{
		glyphs = boldGlyphs;
		atlasIndex = 1.0f;
	}
	else
	{
		glyphs = regularGlyphs;
		atlasIndex = 0.0f;
	}

	float offset;

	for (size_t i = 0; i < length; i++)
	{
		uint32_t value = string[i];

		if (value == '\n')
		{
			switch (alignment)
			{
			default:
				abort();
			case CENTER_ALIGNMENT_TYPE:
				offset = vertexOffsetX * -0.5f;

				for (uint32_t j = lastNewLineIndex; j < vertexIndex; j++)
					vertices[j].position.x += offset;
				break;
			case LEFT_ALIGNMENT_TYPE:
				break;
			case RIGHT_ALIGNMENT_TYPE:
				offset = -vertexOffsetX;

				for (uint32_t j = lastNewLineIndex; j < vertexIndex; j++)
					vertices[j].position.x += offset;
				break;
			case BOTTOM_ALIGNMENT_TYPE:
				offset = vertexOffsetX * -0.5f;

				for (uint32_t j = lastNewLineIndex; j < vertexIndex; j++)
					vertices[j].position.x += offset;
				break;
			case TOP_ALIGNMENT_TYPE:
				offset = vertexOffsetX * -0.5f;

				for (uint32_t j = lastNewLineIndex; j < vertexIndex; j++)
					vertices[j].position.x += offset;
				break;
			case LEFT_BOTTOM_ALIGNMENT_TYPE:
			case LEFT_TOP_ALIGNMENT_TYPE:
				break;
			case RIGHT_BOTTOM_ALIGNMENT_TYPE:
				offset = -vertexOffsetX;

				for (uint32_t j = lastNewLineIndex; j < vertexIndex; j++)
					vertices[j].position.x += offset;
				break;
			case RIGHT_TOP_ALIGNMENT_TYPE:
				offset = -vertexOffsetX;

				for (uint32_t j = lastNewLineIndex; j < vertexIndex; j++)
					vertices[j].position.x += offset;
				break;
			}

			lastNewLineIndex = vertexIndex;

			if (sizeX < vertexOffsetX)
				sizeX = vertexOffsetX;

			vertexOffsetY -= newLineAdvance;
			vertexOffsetX = 0.0f;
			continue;
		}
		else if (value == '\t')
		{
			value = ' ';

			const Glyph* glyph = bsearch(
				&value,
				glyphs,
				glyphCount,
				sizeof(Glyph),
				compareGlyph);

			if (!glyph)
				return false;

			vertexOffsetX += glyph->advance * 4;
			continue;
		}
		else if ((value == '<') & useTags)
		{
			if (i + 2 < length && string[i + 2] == '>')
			{
				uint32_t tag = string[i + 1];

				if (tag == 'b')
				{
					if (useItalic)
					{
						glyphs = boldItalicGlyphs;
						atlasIndex = 3.0f;
					}
					else
					{
						glyphs = boldGlyphs;
						atlasIndex = 1.0f;
					}

					useBold = true;
					i += 2;
					continue;
				}
				else if (tag == 'i')
				{
					if (useBold)
					{
						glyphs = boldItalicGlyphs;
						atlasIndex = 3.0f;
					}
					else
					{
						glyphs = italicGlyphs;
						atlasIndex = 2.0f;
					}

					useItalic = true;
					i += 2;
					continue;
				}
			}
			else if (i + 3 < length && string[i + 1] == '/' && string[i + 3] == '>')
			{
				uint32_t tag = string[i + 2];

				if (tag == 'b')
				{
					if (useItalic)
					{
						glyphs = italicGlyphs;
						atlasIndex = 2.0f;
					}
					else
					{
						glyphs = regularGlyphs;
						atlasIndex = 0.0f;
					}

					useBold = false;
					i += 3;
					continue;
				}
				else if (tag == 'i')
				{
					if (useBold)
					{
						glyphs = boldGlyphs;
						atlasIndex = 1.0f;
					}
					else
					{
						glyphs = regularGlyphs;
						atlasIndex = 0.0f;
					}

					useItalic = false;
					i += 3;
					continue;
				}
				else if (tag == '#')
				{
					useColor = color;
					i += 3;
					continue;
				}
			}
			else if (i + 8 < length && string[i + 1] == '#' && string[i + 8] == '>')
			{
				SrgbColor newColor;

				bool result = hexToColor(
					string + 2,
					&newColor.r);
				result &= hexToColor(
					string + 4,
					&newColor.g);
				result &= hexToColor(
					string + 6,
					&newColor.b);

				if (result)
				{
					newColor.a = UINT8_MAX;
					useColor = newColor;
					i += 8;
					continue;
				}
			}
			else if (i + 10 < length && string[i + 1] == '#' && string[i + 10] == '>')
			{
				SrgbColor newColor;

				bool result = hexToColor(
					string + 2,
					&newColor.r);
				result &= hexToColor(
					string + 4,
					&newColor.g);
				result &= hexToColor(
					string + 6,
					&newColor.b);
				result &= hexToColor(
					string + 8,
					&newColor.a);

				if (result)
				{
					useColor = newColor;
					i += 10;
					continue;
				}
			}
		}

		const Glyph* glyph = bsearch(
			&value,
			glyphs,
			glyphCount,
			sizeof(Glyph),
			compareGlyph);

		if (!glyph)
			return false;

		if (glyph->isVisible)
		{
			float positionX = vertexOffsetX + glyph->positionX;
			float positionY = vertexOffsetY + glyph->positionY;
			float positionZ = vertexOffsetX + glyph->positionZ;
			float positionW = vertexOffsetY + glyph->positionW;
			float texCoordsX = glyph->texCoordsX;
			float texCoordsY = glyph->texCoordsY;
			float texCoordsZ = glyph->texCoordsZ;
			float texCoordsW = glyph->texCoordsW;

			TextVertex vertex;
			vertex.position.x = positionX;
			vertex.position.y = positionY;
			vertex.texCoords.x = texCoordsX;
			vertex.texCoords.y = texCoordsW;
			vertex.texCoords.z = atlasIndex;
			vertex.color = useColor;
			vertices[vertexIndex + 0] = vertex;

			vertex.position.x = positionX;
			vertex.position.y = positionW;
			vertex.texCoords.x = texCoordsX;
			vertex.texCoords.y = texCoordsY;
			vertices[vertexIndex + 1] = vertex;

			vertex.position.x = positionZ;
			vertex.position.y = positionW;
			vertex.texCoords.x = texCoordsZ;
			vertex.texCoords.y = texCoordsY;
			vertices[vertexIndex + 2] = vertex;

			vertex.position.x = positionZ;
			vertex.position.y = positionY;
			vertex.texCoords.x = texCoordsZ;
			vertex.texCoords.y = texCoordsW;
			vertices[vertexIndex + 3] = vertex;

			vertexIndex += 4;
		}

		vertexOffsetX += glyph->advance;
	}

	if (sizeX < vertexOffsetX)
		sizeX = vertexOffsetX;
	sizeY = -vertexOffsetY;

	switch (alignment)
	{
	default:
		abort();
	case CENTER_ALIGNMENT_TYPE:
		offset = vertexOffsetX * -0.5f;

		for (uint32_t i = lastNewLineIndex; i < vertexIndex; i++)
			vertices[i].position.x += offset;

		offset = sizeY * 0.5f;

		for (uint32_t i = 0; i < vertexIndex; i++)
			vertices[i].position.y += offset;
		break;
	case LEFT_ALIGNMENT_TYPE:
		offset = sizeY * 0.5f;

		for (uint32_t i = 0; i < vertexIndex; i++)
			vertices[i].position.y += offset;
		break;
	case RIGHT_ALIGNMENT_TYPE:
		offset = -vertexOffsetX;

		for (uint32_t i = lastNewLineIndex; i < vertexIndex; i++)
			vertices[i].position.x += offset;

		offset = sizeY * 0.5f;

		for (uint32_t i = 0; i < vertexIndex; i++)
			vertices[i].position.y += offset;
		break;
	case BOTTOM_ALIGNMENT_TYPE:
		offset = vertexOffsetX * -0.5f;

		for (uint32_t i = lastNewLineIndex; i < vertexIndex; i++)
			vertices[i].position.x += offset;

		offset = sizeY;

		for (uint32_t i = 0; i < vertexIndex; i++)
			vertices[i].position.y += offset;
		break;
	case TOP_ALIGNMENT_TYPE:
		offset = vertexOffsetX * -0.5f;

		for (uint32_t i = lastNewLineIndex; i < vertexIndex; i++)
			vertices[i].position.x += offset;
		break;
	case LEFT_BOTTOM_ALIGNMENT_TYPE:
		offset = sizeY;

		for (uint32_t i = 0; i < vertexIndex; i++)
			vertices[i].position.y += offset;
		break;
	case LEFT_TOP_ALIGNMENT_TYPE:
		break;
	case RIGHT_BOTTOM_ALIGNMENT_TYPE:
		offset = -vertexOffsetX;

		for (uint32_t i = lastNewLineIndex; i < vertexIndex; i++)
			vertices[i].position.x += offset;

		offset = sizeY;

		for (uint32_t i = 0; i < vertexIndex; i++)
			vertices[i].position.y += offset;
		break;
	case RIGHT_TOP_ALIGNMENT_TYPE:
		offset = -vertexOffsetX;

		for (uint32_t i = lastNewLineIndex; i < vertexIndex; i++)
			vertices[i].position.x += offset;
		break;
	}

	*vertexCount = vertexIndex;
	*textSize = vec2F(sizeX, sizeY + newLineAdvance * 0.25f);
	return true;
}
inline static uint32_t* createIndices(uint32_t indexCount)
{
	assert(indexCount > 0);

	uint32_t* indices = malloc(
		indexCount * sizeof(uint32_t));

	if (!indices)
		return NULL;

	for (uint32_t i = 0, j = 0; i < indexCount; i += 6, j += 4)
	{
		indices[i + 0] = (uint32_t)j + 0;
		indices[i + 1] = (uint32_t)j + 1;
		indices[i + 2] = (uint32_t)j + 2;
		indices[i + 3] = (uint32_t)j + 0;
		indices[i + 4] = (uint32_t)j + 2;
		indices[i + 5] = (uint32_t)j + 3;
	}

	return indices;
}

inline static void internalDestroyText(Text text)
{
	if (!text)
		return;

	GraphicsAPI api = getGraphicsAPI();

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		destroyBuffer(text->vk.vertexBuffer);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		GraphicsMesh mesh = text->gl.mesh;

		if (mesh)
		{
			Buffer vertexBuffer = getGraphicsMeshVertexBuffer(mesh);
			destroyGraphicsMesh(mesh);
			destroyBuffer(vertexBuffer);
		}
#else
		abort();
#endif
	}
	else
	{
		abort();
	}

	free(text->base.string);
	free(text);
}
inline static MpgxResult internalCreateText(
	FontAtlas fontAtlas,
	uint32_t* string,
	size_t length,
	size_t capacity,
	AlignmentType alignment,
	SrgbColor color,
	bool isBold,
	bool isItalic,
	bool useTags,
	bool isConstant,
	Text* text)
{
	assert(fontAtlas);
	assert(capacity > 0);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(text);
	assert(textInitialized);

	assert(length == 0 ||
		(length > 0 && string));

	Text textInstance = calloc(
		1, sizeof(Text_T));

	if (!textInstance)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	textInstance->base.fontAtlas = fontAtlas;
	textInstance->base.string = string;
	textInstance->base.capacity = capacity;
	textInstance->base.length = length;
	textInstance->base.color = color;
	textInstance->base.alignment = alignment;
	textInstance->base.isBold = isBold;
	textInstance->base.isItalic = isItalic;
	textInstance->base.useTags = useTags;
	textInstance->base.isConstant = isConstant;

	GraphicsPipeline pipeline = fontAtlas->pipeline;
	Handle handle = pipeline->base.handle;
	TextVertex* vertexBuffer = handle->base.vertexBuffer;
	size_t vertexCapacity = handle->base.vertexCapacity;

	if (vertexCapacity < length * 4)
	{
		capacity = length * 4;
		TextVertex* newVertexBuffer;

		if (vertexBuffer)
		{
			newVertexBuffer = realloc(
				vertexBuffer,
				capacity * sizeof(TextVertex));
		}
		else
		{
			newVertexBuffer = malloc(
				capacity * sizeof(TextVertex));
		}

		if (!newVertexBuffer)
		{
			internalDestroyText(textInstance);
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		}

		handle->base.vertexBuffer = vertexBuffer = newVertexBuffer;
		handle->base.vertexCapacity = capacity;
	}

	uint32_t vertexCount;

	bool result = fillVertices(
		string,
		length,
		fontAtlas->regularGlyphs,
		fontAtlas->boldGlyphs,
		fontAtlas->italicGlyphs,
		fontAtlas->boldItalicGlyphs,
		fontAtlas->glyphCount,
		fontAtlas->newLineAdvance,
		alignment,
		color,
		isBold,
		isItalic,
		useTags,
		vertexBuffer,
		&vertexCount,
		&textInstance->base.size);

	if (!result)
	{
		internalDestroyText(textInstance);
		return BAD_VALUE_MPGX_RESULT;
	}

	Window window = pipeline->base.window;
	Buffer vertexBufferInstance;

	if (vertexCount > 0)
	{
		MpgxResult mpgxResult = createBuffer(window,
			VERTEX_BUFFER_TYPE,
			isConstant ? GPU_ONLY_BUFFER_USAGE : CPU_TO_GPU_BUFFER_USAGE,
			vertexBuffer,
			vertexCount * sizeof(TextVertex),
			&vertexBufferInstance);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			internalDestroyText(textInstance);
			return mpgxResult;
		}
	}
	else
	{
		vertexBufferInstance = NULL;
	}

	GraphicsAPI api = getGraphicsAPI();
	Buffer indexBuffer = handle->base.indexBuffer;
	Text* texts = handle->base.texts;
	size_t textCount = handle->base.textCount;
	uint32_t indexCount = (vertexCount / 4) * 6;
	size_t indexSize = indexCount * sizeof(uint32_t);

	if (!indexBuffer || indexBuffer->base.size < indexSize)
	{
		uint32_t* indices = createIndices(indexCount);

		if (!indices)
		{
			destroyBuffer(vertexBufferInstance);
			internalDestroyText(textInstance);
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		}

		Buffer newIndexBuffer;

		MpgxResult mpgxResult = createBuffer(window,
			INDEX_BUFFER_TYPE,
			GPU_ONLY_BUFFER_USAGE,
			indices,
			indexSize,
			&newIndexBuffer);

		free(indices);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			destroyBuffer(vertexBufferInstance);
			internalDestroyText(textInstance);
			return mpgxResult;
		}

		if (api == OPENGL_GRAPHICS_API ||
			api == OPENGL_ES_GRAPHICS_API)
		{
#if MPGX_SUPPORT_OPENGL
			for (size_t i = 0; i < textCount; i++)
			{
				GraphicsMesh textMesh = texts[i]->gl.mesh;
				textMesh->gl.indexBuffer = newIndexBuffer;
			}
#else
			abort();
#endif
		}

		handle->base.indexBuffer = newIndexBuffer;
		destroyBuffer(indexBuffer);
	}

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		textInstance->vk.vertexBuffer = vertexBufferInstance;
		textInstance->vk.indexCount = indexCount;
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		GraphicsMesh mesh;

		MpgxResult mpgxResult = createGraphicsMesh(
			window,
			UINT32_INDEX_TYPE,
			indexCount,
			0,
			vertexBufferInstance,
			handle->base.indexBuffer,
			&mesh);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			destroyBuffer(vertexBufferInstance);
			internalDestroyText(textInstance);
			return mpgxResult;
		}

		textInstance->gl.mesh = mesh;
#else
		abort();
#endif
	}
	else
	{
		abort();
	}

	if (textCount == handle->base.textCapacity)
	{
		capacity = handle->base.textCapacity * 2;

		Text* newTexts = realloc(texts,
			sizeof(Text) * capacity);

		if (!newTexts)
		{
			internalDestroyText(textInstance);
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		}

		handle->base.texts = newTexts;
		handle->base.textCapacity = capacity;
	}

	handle->base.texts[textCount] = textInstance;
	handle->base.textCount = textCount + 1;

	*text = textInstance;
	return SUCCESS_MPGX_RESULT;
}

MpgxResult createAtlasText(
	FontAtlas fontAtlas,
	const uint32_t* string,
	size_t length,
	AlignmentType alignment,
	SrgbColor color,
	bool isBold,
	bool isItalic,
	bool useTags,
	bool isConstant,
	Text* text)
{
	assert(fontAtlas);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(text);
	assert(textInitialized);

	assert(length == 0 ||
		(length > 0 && string));

	uint32_t* stringArray;
	size_t capacity;

	if (length > 0)
	{
		stringArray = malloc(length * sizeof(uint32_t));

		if (!stringArray)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;

		memcpy(stringArray, string,
			sizeof(uint32_t) * length);
		capacity = length;
	}
	else
	{
		stringArray = malloc(sizeof(uint32_t));

		if (!stringArray)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;

		capacity = 1;
	}

	return internalCreateText(
		fontAtlas,
		stringArray,
		length,
		capacity,
		alignment,
		color,
		isBold,
		isItalic,
		useTags,
		isConstant,
		text);
}
MpgxResult createAtlasText8(
	FontAtlas fontAtlas,
	const char* string,
	size_t length,
	AlignmentType alignment,
	SrgbColor color,
	bool isBold,
	bool isItalic,
	bool useTags,
	bool isConstant,
	Text* text)
{
	assert(fontAtlas);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(text);
	assert(textInitialized);

	assert(length == 0 ||
		(length > 0 && string));

	uint32_t* string32;
	size_t length32;
	size_t capacity;

	if (length > 0)
	{
		MpgxResult mpgxResult = allocateStringUTF32(
			string,
			length,
			&string32,
			&length32);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
			return mpgxResult;

		capacity = length32;
	}
	else
	{
		string32 = malloc(sizeof(uint32_t));

		if (!string32)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;

		length32 = 0;
		capacity = 1;
	}

	return internalCreateText(
		fontAtlas,
		string32,
		length32,
		capacity,
		alignment,
		color,
		isBold,
		isItalic,
		useTags,
		isConstant,
		text);
}

void destroyText(Text text)
{
	if (!text)
		return;

	assert(textInitialized);

	FontAtlas fontAtlas = text->base.fontAtlas;
	Handle handle = fontAtlas->pipeline->base.handle;
	Text* texts = handle->base.texts;
	size_t textCount = handle->base.textCount;

	for (size_t i = 0; i < textCount; i++)
	{
		if (texts[i] != text)
			continue;

		for (size_t j = i + 1; j < textCount; j++)
			texts[j - 1] = texts[j];

		internalDestroyText(text);
		handle->base.textCount--;
		return;
	}

	abort();
}

FontAtlas getTextFontAtlas(Text text)
{
	assert(text);
	assert(textInitialized);
	return text->base.fontAtlas;
}
Vec2F getTextSize(Text text)
{
	assert(text);
	assert(textInitialized);
	return text->base.size;
}
bool isTextConstant(Text text)
{
	assert(text);
	assert(textInitialized);
	return text->base.isConstant;
}

const uint32_t* getTextString(Text text)
{
	assert(text);
	assert(textInitialized);
	return text->base.string;
}
size_t getTextLength(Text text)
{
	assert(text);
	assert(textInitialized);
	return text->base.length;
}

bool setTextString(
	Text text,
	const uint32_t* string,
	size_t length)
{
	assert(text);
	assert(textInitialized);
	assert(!text->base.isConstant);

	assert(length == 0 ||
		(length > 0 && string));

	if (length > text->base.capacity)
	{
		uint32_t* newString = realloc(
			text->base.string,
			length * sizeof(uint32_t));

		if (!newString)
			return false;

		text->base.string = newString;
		text->base.capacity = length;
	}

	memcpy(text->base.string, string,
		length * sizeof(uint32_t));
	text->base.length = length;
	return true;
}
bool setTextString8(
	Text text,
	const char* string,
	size_t length)
{
	assert(text);
	assert(textInitialized);
	assert(!text->base.isConstant);

	assert(length == 0 ||
		(length > 0 && string));

	if (length > text->base.capacity)
	{
		uint32_t* newString = realloc(
			text->base.string,
			length * sizeof(uint32_t));

		if (!newString)
			return false;

		text->base.string = newString;
		text->base.capacity = length;
	}

	if (length > 0)
	{
		size_t newLength = stringUTF8toUTF32(
			string,
			length,
			text->base.string);

		text->base.length = newLength;
	}
	else
	{
		text->base.length = 0;
	}

	return true;
}

bool appendTextString32(
	Text text,
	const uint32_t* string,
	size_t length,
	size_t index)
{
	assert(text);
	assert(string);
	assert(length > 0);
	assert(index <= text->base.length);

	assert((size_t)text->base.length +
		(size_t)length <= UINT32_MAX);

	size_t baseLength = text->base.length;

	if (baseLength + length > text->base.capacity)
	{
		size_t capacity = baseLength + length;

		uint32_t* newString = realloc(
			text->base.string,
			capacity * sizeof(uint32_t));

		if (!newString)
			return false;

		text->base.string = newString;
		text->base.capacity = capacity;
	}

	uint32_t* baseString = text->base.string;

	if (index < baseLength)
	{
		size_t offset = index + length;

		for (int64_t i = (int64_t)baseLength - (int64_t)(index + 1); i >= 0; i--)
			baseString[offset + i] = baseString[index + i];
	}

	memcpy(baseString + index, string,
		length * sizeof(uint32_t));
	text->base.length += length;
	return true;
}
void removeTextChar(
	Text text,
	size_t index)
{
	assert(text);
	assert(index < text->base.length);

	uint32_t* string = text->base.string;
	size_t length = text->base.length;

	for (size_t i = index + 1; i < length; i++)
		string[i - 1] = string[i];

	text->base.length--;
}

AlignmentType getTextAlignment(Text text)
{
	assert(text);
	assert(textInitialized);
	return text->base.alignment;
}
void setTextAlignment(
	Text text,
	AlignmentType alignment)
{
	assert(text);
	assert(textInitialized);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(!text->base.isConstant);
	text->base.alignment = alignment;
}

SrgbColor getTextColor(Text text)
{
	assert(text);
	assert(textInitialized);
	return text->base.color;
}
void setTextColor(
	Text text,
	SrgbColor color)
{
	assert(text);
	assert(textInitialized);
	assert(!text->base.isConstant);
	text->base.color = color;
}

bool isTextBold(Text text)
{
	assert(text);
	assert(textInitialized);
	return text->base.isBold;
}
void setTextBold(
	Text text,
	bool isBold)
{
	assert(text);
	assert(textInitialized);
	assert(!text->base.isConstant);
	text->base.isBold = isBold;
}

bool isTextItalic(Text text)
{
	assert(text);
	assert(textInitialized);
	return text->base.isItalic;
}
void setTextItalic(
	Text text,
	bool isItalic)
{
	assert(text);
	assert(textInitialized);
	assert(!text->base.isConstant);
	text->base.isItalic = isItalic;
}

bool isTextUseTags(Text text)
{
	assert(text);
	assert(textInitialized);
	return text->base.useTags;
}
void setTextUseTags(
	Text text,
	bool useTags)
{
	assert(text);
	assert(textInitialized);
	assert(!text->base.isConstant);
	text->base.useTags = useTags;
}

bool getTextCursorAdvance(
	Text text,
	size_t index,
	Vec2F* _advance)
{
	assert(text);
	assert(index <= text->base.length);
	assert(_advance);
	assert(textInitialized);

	FontAtlas fontAtlas = text->base.fontAtlas;
	const Glyph* regularGlyphs = fontAtlas->regularGlyphs;
	const Glyph* boldGlyphs = fontAtlas->boldGlyphs;
	const Glyph* italicGlyphs = fontAtlas->italicGlyphs;
	const Glyph* boldItalicGlyphs = fontAtlas->boldItalicGlyphs;
	size_t glyphCount = fontAtlas->glyphCount;
	float newLineAdvance = fontAtlas->newLineAdvance;
	const uint32_t* string = text->base.string;
	size_t length = text->base.length;
	bool useBold = text->base.isBold;
	bool useItalic = text->base.isItalic;
	bool useTags = text->base.useTags;

	const Glyph* glyphs = regularGlyphs;
	Vec2F advance = zeroVec2F;
	float lineSizeX = 0.0f;

	for (size_t i = 0; i < length; i++)
	{
		uint32_t value = string[i];

		if (value == '\n')
		{
			if (i >= index)
				break;

			advance.y -= newLineAdvance;
			advance.x = lineSizeX = 0.0f;
			continue;
		}
		else if (value == '\t')
		{
			value = ' ';

			const Glyph* glyph = bsearch(
				&value,
				glyphs,
				glyphCount,
				sizeof(Glyph),
				compareGlyph);

			if (!glyph)
				return false;

			advance.x += glyph->advance;
			continue;
		}
		else if ((value == '<') & useTags)
		{
			if (i + 2 < length && string[i + 2] == '>')
			{
				uint32_t tag = string[i + 1];

				if (tag == 'b')
				{
					if (useItalic) glyphs = boldItalicGlyphs;
					else glyphs = boldGlyphs;
					useBold = true;
					i += 2;
					continue;
				}
				else if (tag == 'i')
				{
					if (useBold) glyphs = boldItalicGlyphs;
					else glyphs = italicGlyphs;
					useItalic = true;
					i += 2;
					continue;
				}
			}
			else if (i + 3 < length && string[i + 1] == '/' && string[i + 3] == '>')
			{
				uint32_t tag = string[i + 2];

				if (tag == 'b')
				{
					if (useItalic) glyphs = italicGlyphs;
					else glyphs = regularGlyphs;
					useBold = false;
					i += 3;
					continue;
				}
				else if (tag == 'i')
				{
					if (useBold) glyphs = boldGlyphs;
					else glyphs = regularGlyphs;
					useItalic = false;
					i += 3;
					continue;
				}
				else if (tag == '#')
				{
					i += 3;
					continue;
				}
			}
			else if (i + 8 < length && string[i + 1] == '#' && string[i + 8] == '>')
			{
				i += 8;
				continue;
			}
			else if (i + 10 < length && string[i + 1] == '#' && string[i + 10] == '>')
			{
				i += 10;
				continue;
			}
		}

		const Glyph* glyph = bsearch(
			&value,
			glyphs,
			glyphCount,
			sizeof(Glyph),
			compareGlyph);

		if (!glyph)
			return false;

		if (i < index)
			advance.x += glyph->advance;
		lineSizeX += glyph->advance;
	}

	AlignmentType alignment = text->base.alignment;
	Vec2F size = text->base.size;

	switch (alignment)
	{
	default:
		abort();
	case CENTER_ALIGNMENT_TYPE:
		advance.x -= lineSizeX * (cmmt_float_t)0.5;
		advance.y += (size.y - (newLineAdvance * 0.5f +
			newLineAdvance * 0.25f)) * (cmmt_float_t)0.5;
		break;
	case LEFT_ALIGNMENT_TYPE:
		advance.y += (size.y - (newLineAdvance * 0.5f +
			newLineAdvance * 0.25f)) * (cmmt_float_t)0.5;
		break;
	case RIGHT_ALIGNMENT_TYPE:
		advance.x -= lineSizeX;
		advance.y += (size.y - (newLineAdvance * 0.5f +
			newLineAdvance * 0.25f)) * (cmmt_float_t)0.5;
		break;
	case BOTTOM_ALIGNMENT_TYPE:
		advance.x -= lineSizeX * (cmmt_float_t)0.5;
		advance.y += size.y - newLineAdvance * 0.5f;
		break;
	case TOP_ALIGNMENT_TYPE:
		advance.x -= lineSizeX * (cmmt_float_t)0.5;
		advance.y -= newLineAdvance * 0.25f;
		break;
	case LEFT_BOTTOM_ALIGNMENT_TYPE:
		advance.y += size.y - newLineAdvance * 0.5f;
		break;
	case LEFT_TOP_ALIGNMENT_TYPE:
		advance.y -= newLineAdvance * 0.25f;
		break;
	case RIGHT_BOTTOM_ALIGNMENT_TYPE:
		advance.x -= lineSizeX;
		advance.y += size.y - newLineAdvance * 0.5f;
		break;
	case RIGHT_TOP_ALIGNMENT_TYPE:
		advance.x -= lineSizeX;
		advance.y -= newLineAdvance * 0.25f;
		break;
	}

	*_advance = advance;
	return true;
}
bool getTextCursorIndex(
	Text text,
	Vec2F advance,
	size_t* _index)
{
	assert(text);
	assert(_index);
	assert(textInitialized);

	size_t length = text->base.length;

	cmmt_float_t bestDistance = INFINITY;
	size_t index = 0;

	// TODO: too heavy, use better solution
	for (size_t i = 0; i <= length; i++)
	{
		Vec2F checkAdvance;

		bool result = getTextCursorAdvance(
			text,
			i,
			&checkAdvance);

		if (!result)
			return false;

		float distance = distPowVec2F(
			advance, checkAdvance);

		if (distance < bestDistance)
		{
			bestDistance = distance;
			index = i;
		}
	}

	*_index = index;
	return true;
}

MpgxResult bakeText(Text text)
{
	assert(text);
	assert(!text->base.isConstant);
	assert(textInitialized);

	FontAtlas fontAtlas = text->base.fontAtlas;
	size_t length = text->base.length;
	GraphicsPipeline pipeline = fontAtlas->pipeline;
	Handle handle = pipeline->base.handle;
	TextVertex* vertexBuffer = handle->base.vertexBuffer;
	size_t vertexCapacity = handle->base.vertexCapacity;

	if (vertexCapacity < length * 4)
	{
		size_t capacity = length * 4;
		TextVertex* newVertexBuffer;

		if (vertexBuffer)
		{
			newVertexBuffer = realloc(
				vertexBuffer,
				capacity * sizeof(TextVertex));
		}
		else
		{
			newVertexBuffer = malloc(
				capacity * sizeof(TextVertex));
		}

		if (!newVertexBuffer)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;

		handle->base.vertexBuffer = vertexBuffer = newVertexBuffer;
		handle->base.vertexCapacity = capacity;
	}

	uint32_t vertexCount;
	Vec2F textSize;

	bool result = fillVertices(
		text->base.string,
		text->base.length,
		fontAtlas->regularGlyphs,
		fontAtlas->boldGlyphs,
		fontAtlas->italicGlyphs,
		fontAtlas->boldItalicGlyphs,
		fontAtlas->glyphCount,
		fontAtlas->newLineAdvance,
		text->base.alignment,
		text->base.color,
		text->base.isBold,
		text->base.isItalic,
		text->base.useTags,
		vertexBuffer,
		&vertexCount,
		&textSize);

	if (!result)
		return BAD_VALUE_MPGX_RESULT;

	GraphicsAPI api = getGraphicsAPI();
	Window window = pipeline->base.window;
	Buffer indexBuffer = handle->base.indexBuffer;
	Text* texts = handle->base.texts;
	size_t textCount = handle->base.textCount;
	uint32_t indexCount = (vertexCount / 4) * 6;
	size_t indexSize = indexCount * sizeof(uint32_t);

	if (!indexBuffer || indexBuffer->base.size < indexSize)
	{
		uint32_t* indices = createIndices(indexCount);

		if (!indices)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;

		Buffer newIndexBuffer;

		MpgxResult mpgxResult = createBuffer(window,
			INDEX_BUFFER_TYPE,
			GPU_ONLY_BUFFER_USAGE,
			indices,
			indexSize,
			&newIndexBuffer);

		free(indices);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
			return mpgxResult;

		if (api == OPENGL_GRAPHICS_API ||
			api == OPENGL_ES_GRAPHICS_API)
		{
#if MPGX_SUPPORT_OPENGL
			for (size_t i = 0; i < textCount; i++)
			{
				GraphicsMesh textMesh = texts[i]->gl.mesh;
				textMesh->gl.indexBuffer = newIndexBuffer;
			}
#else
			abort();
#endif
		}

		handle->base.indexBuffer = newIndexBuffer;
		destroyBuffer(indexBuffer);
	}

	Buffer vertexBufferInstance;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		vertexBufferInstance = text->vk.vertexBuffer;
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		vertexBufferInstance = text->gl.mesh->gl.vertexBuffer;
#else
		abort();
#endif
	}
	else
	{
		abort();
	}

	size_t vertexSize = vertexCount * sizeof(TextVertex);

	if (vertexBufferInstance->base.size < vertexSize)
	{
		Buffer newVertexBuffer;

		MpgxResult mpgxResult = createBuffer(window,
			VERTEX_BUFFER_TYPE,
			CPU_TO_GPU_BUFFER_USAGE,
			vertexBuffer,
			vertexSize,
			&newVertexBuffer);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
			return mpgxResult;

		if (api == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			text->vk.vertexBuffer = newVertexBuffer;
			text->vk.indexCount = indexCount;
#else
			abort();
#endif
		}
		else
		{
#if MPGX_SUPPORT_OPENGL
			GraphicsMesh mesh = text->gl.mesh;
			mesh->gl.vertexBuffer = newVertexBuffer;
			mesh->gl.indexCount = indexCount;
#else
			abort();
#endif
		}

		destroyBuffer(vertexBufferInstance);
	}
	else
	{
		if (api == VULKAN_GRAPHICS_API)
		{
#if MPGX_SUPPORT_VULKAN
			if (vertexSize > 0)
			{
				VkWindow vkWindow = getVkWindow(window);

				VkResult vkResult = vkQueueWaitIdle(
					vkWindow->graphicsQueue);

				if (vkResult != VK_SUCCESS)
					return vkToMpgxResult(vkResult);

				MpgxResult mpgxResult = setVkBufferData(
					vkWindow->allocator,
					vertexBufferInstance->vk.allocation,
					vertexBuffer,
					vertexSize,
					0);

				if (mpgxResult != SUCCESS_MPGX_RESULT)
					return mpgxResult;
			}

			text->vk.indexCount = indexCount;
#else
			abort();
#endif
		}
		else
		{
#if MPGX_SUPPORT_OPENGL
			if (vertexSize > 0)
			{
				MpgxResult mpgxResult = setGlBufferData(
					vertexBufferInstance->gl.glType,
					vertexBufferInstance->gl.handle,
					vertexBuffer,
					vertexSize,
					0);

				if (mpgxResult != SUCCESS_MPGX_RESULT)
					return mpgxResult;
			}

			text->gl.mesh->gl.indexCount = indexCount;
#else
			abort();
#endif
		}
	}

	text->base.size = textSize;
	return SUCCESS_MPGX_RESULT;
}
size_t drawText(
	Text text,
	Vec4I scissor)
{
	assert(text);
	assert(scissor.x >= 0);
	assert(scissor.y >= 0);
	assert(scissor.z >= 0);
	assert(scissor.w >= 0);
	assert(textInitialized);

	FontAtlas fontAtlas = text->base.fontAtlas;
	GraphicsPipeline pipeline = fontAtlas->pipeline;
	Vec2I framebufferSize = pipeline->base.framebuffer->base.size;

	assert(scissor.x + scissor.z <= framebufferSize.x);
	assert(scissor.y + scissor.w <= framebufferSize.y);

	if (scissor.z + scissor.w == 0)
		scissor = vec4I(0, 0, framebufferSize.x, framebufferSize.y);

	Vec4I stateScissor = pipeline->base.state.scissor;
	Window window = pipeline->base.window;
	bool dynamicScissor = stateScissor.z + stateScissor.w == 0;
	GraphicsAPI api = getGraphicsAPI();

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		uint32_t indexCount = text->vk.indexCount;

		if (indexCount == 0)
			return 0;

		VkWindow vkWindow = getVkWindow(window);
		VkCommandBuffer commandBuffer = vkWindow->currenCommandBuffer;
		VkPipelineLayout pipelineLayout = pipeline->vk.layout;

		if (dynamicScissor)
		{
			VkRect2D vkScissor = {
				(int32_t)scissor.x,
				(int32_t)((framebufferSize.y - scissor.y) - scissor.w),
				(uint32_t)scissor.z,
				(uint32_t)scissor.w,
			};
			vkCmdSetScissor(
				commandBuffer,
				0,
				1,
				&vkScissor);
		}

		Handle handle = pipeline->vk.handle;
		const VkDeviceSize offset = 0;

		vkCmdPushConstants(
			commandBuffer,
			pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(VertexPushConstants),
			&handle->vk.vpc);
		vkCmdPushConstants(
			commandBuffer,
			pipelineLayout,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			sizeof(VertexPushConstants),
			sizeof(FragmentPushConstants),
			&handle->vk.fpc);
		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0,
			1,
			&fontAtlas->descriptorSet,
			0,
			NULL);
		vkCmdBindVertexBuffers(
			commandBuffer,
			0,
			1,
			&text->vk.vertexBuffer->vk.handle,
			&offset);
		vkCmdDrawIndexed(
			commandBuffer,
			indexCount,
			1,
			0,
			0,
			0);
		return indexCount;
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		if (dynamicScissor)
		{
			glScissor(
				(GLint)scissor.x,
				(GLint)scissor.y,
				(GLsizei)scissor.z,
				(GLsizei)scissor.w);
		}

		glBindTexture(
			GL_TEXTURE_2D,
			fontAtlas->atlasImage->gl.handle);
		assertOpenGL();

		return drawGraphicsMesh(
			pipeline,
			text->gl.mesh);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}
}

MpgxResult createTextSampler(
	Window window,
	Sampler* textSampler)
{
	assert(window);
	assert(textSampler);

	return createSampler(window,
		NEAREST_IMAGE_FILTER,
		NEAREST_IMAGE_FILTER,
		NEAREST_IMAGE_FILTER,
		false,
		REPEAT_IMAGE_WRAP,
		REPEAT_IMAGE_WRAP,
		REPEAT_IMAGE_WRAP,
		NEVER_COMPARE_OPERATOR,
		false,
		defaultMipmapLodRange,
		DEFAULT_MIPMAP_LOD_BIAS,
		textSampler);
}

#if MPGX_SUPPORT_VULKAN
static const VkVertexInputBindingDescription vertexInputBindingDescriptions[1] = {
	{
		0,
		sizeof(TextVertex),
		VK_VERTEX_INPUT_RATE_VERTEX,
	},
};
static const VkVertexInputAttributeDescription vertexInputAttributeDescriptions[3] = {
	{
		0,
		0,
		VK_FORMAT_R32G32_SFLOAT,
		0,
	},
	{
		1,
		0,
		VK_FORMAT_R32G32B32_SFLOAT,
		sizeof(Vec2F),
	},
	{
		2,
		0,
		VK_FORMAT_R8G8B8A8_UINT,
		sizeof(Vec2F) + sizeof(Vec3F),
	},
};
static const VkPushConstantRange pushConstantRanges[2] = {
	{
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		sizeof(VertexPushConstants),
	},
	{
		VK_SHADER_STAGE_FRAGMENT_BIT,
		sizeof(VertexPushConstants),
		sizeof(FragmentPushConstants),
	},
};

static void onVkBind(GraphicsPipeline graphicsPipeline)
{
	assert(graphicsPipeline);

	Handle handle = graphicsPipeline->vk.handle;
	VkWindow vkWindow = getVkWindow(graphicsPipeline->vk.window);

	if (handle->vk.indexBuffer)
	{
		vkCmdBindIndexBuffer(
			vkWindow->currenCommandBuffer,
			handle->vk.indexBuffer->vk.handle,
			0,
			VK_INDEX_TYPE_UINT32);
	}
}
static void onVkResize(
	GraphicsPipeline graphicsPipeline,
	Vec2I newSize,
	void* createData)
{
	assert(graphicsPipeline);
	assert(newSize.x > 0);
	assert(newSize.y > 0);
	assert(createData);

	Handle handle = graphicsPipeline->vk.handle;

	Vec4I size = vec4I(0, 0,
		newSize.x, newSize.y);

	if (graphicsPipeline->vk.state.viewport.z +
		graphicsPipeline->vk.state.viewport.w)
	{
		graphicsPipeline->vk.state.viewport = size;
	}
	if (graphicsPipeline->vk.state.scissor.z +
		graphicsPipeline->vk.state.scissor.w)
	{
		graphicsPipeline->vk.state.scissor = size;
	}

	VkGraphicsPipelineCreateData _createData = {
		1,
		vertexInputBindingDescriptions,
		3,
		vertexInputAttributeDescriptions,
		1,
		&handle->vk.descriptorSetLayout,
		2,
		pushConstantRanges,
	};

	*(VkGraphicsPipelineCreateData*)createData = _createData;
}
static void onVkDestroy(
	Window window,
	void* _handle)
{
	assert(window);
	Handle handle = (Handle)_handle;

	if (!handle)
		return;

	assert(handle->vk.textCount == 0);

	VkWindow vkWindow = getVkWindow(window);
	VkDevice device = vkWindow->device;

	destroyBuffer(handle->vk.indexBuffer);
	vkDestroyDescriptorSetLayout(
		device,
		handle->vk.descriptorSetLayout,
		NULL);
	free(handle->vk.vertexBuffer);
	free(handle->vk.texts);
	free(handle);
}
inline static MpgxResult createVkPipeline(
	Framebuffer framebuffer,
	Window window,
	const char* name,
	const GraphicsPipelineState* state,
	Handle handle,
	Shader* shaders,
	uint8_t shaderCount,
	GraphicsPipeline* graphicsPipeline)
{
	assert(framebuffer);
	assert(window);
	assert(state);
	assert(handle);
	assert(shaders);
	assert(shaderCount > 0);
	assert(graphicsPipeline);

	VkWindow vkWindow = getVkWindow(framebuffer->vk.window);
	VkDevice device = vkWindow->device;

	VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[1] = {
		{
			0,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			NULL,
		},
	};
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		NULL,
		0,
		1,
		descriptorSetLayoutBindings
	};

	VkDescriptorSetLayout descriptorSetLayout;

	VkResult vkResult = vkCreateDescriptorSetLayout(
		device,
		&descriptorSetLayoutCreateInfo,
		NULL,
		&descriptorSetLayout);

	if(vkResult != VK_SUCCESS)
	{
		onVkDestroy(window, handle);
		return vkToMpgxResult(vkResult);
	}

	VkGraphicsPipelineCreateData createData = {
		1,
		vertexInputBindingDescriptions,
		3,
		vertexInputAttributeDescriptions,
		1,
		&descriptorSetLayout,
		2,
		pushConstantRanges,
	};

	handle->vk.descriptorSetLayout = descriptorSetLayout;

	return createGraphicsPipeline(
		framebuffer,
		name,
		state,
		onVkBind,
		NULL,
		onVkResize,
		onVkDestroy,
		handle,
		&createData,
		shaders,
		shaderCount,
		graphicsPipeline);
}
#endif

#if MPGX_SUPPORT_OPENGL
static void onGlBind(GraphicsPipeline graphicsPipeline)
{
	assert(graphicsPipeline);
	Handle handle = graphicsPipeline->gl.handle;

	glUniform1i(handle->gl.atlasLocation, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindSampler(0, handle->gl.sampler->gl.handle);

	assertOpenGL();
}
static void onGlUniformsSet(GraphicsPipeline graphicsPipeline)
{
	assert(graphicsPipeline);
	Handle handle = graphicsPipeline->gl.handle;

	glUniformMatrix4fv(
		handle->gl.mvpLocation,
		1,
		GL_FALSE,
		(const float*)&handle->gl.vpc.mvp);
	glUniform4fv(
		handle->gl.colorLocation,
		1,
		(const GLfloat*)&handle->gl.fpc.color);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(
		0,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(TextVertex),
		0);
	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(TextVertex),
		(const void*)sizeof(Vec2F));
	glVertexAttribIPointer(
		2,
		4,
		GL_UNSIGNED_BYTE,
		sizeof(TextVertex),
		(const void*)(sizeof(Vec2F) + sizeof(Vec3F)));
	assertOpenGL();
}
static void onGlResize(
	GraphicsPipeline graphicsPipeline,
	Vec2I newSize,
	void* createData)
{
	assert(graphicsPipeline);
	assert(newSize.x > 0);
	assert(newSize.y > 0);
	assert(!createData);

	Vec4I size = vec4I(0, 0,
		newSize.x, newSize.y);

	if (graphicsPipeline->gl.state.viewport.z +
		graphicsPipeline->gl.state.viewport.w)
	{
		graphicsPipeline->gl.state.viewport = size;
	}
	if (graphicsPipeline->gl.state.scissor.z +
		graphicsPipeline->gl.state.scissor.w)
	{
		graphicsPipeline->gl.state.scissor = size;
	}
}
static void onGlDestroy(
	Window window,
	void* _handle)
{
	assert(window);
	Handle handle = (Handle)_handle;

	if (!handle)
		return;

	assert(handle->gl.textCount == 0);

	destroyBuffer(handle->gl.indexBuffer);
	free(handle->gl.vertexBuffer);
	free(handle->gl.texts);
	free(handle);
}
inline static MpgxResult createGlPipeline(
	Framebuffer framebuffer,
	Window window,
	const char* name,
	const GraphicsPipelineState* state,
	Handle handle,
	Shader* shaders,
	uint8_t shaderCount,
	GraphicsPipeline* graphicsPipeline)
{
	assert(framebuffer);
	assert(window);
	assert(state);
	assert(handle);
	assert(shaders);
	assert(shaderCount > 0);
	assert(graphicsPipeline);

	GraphicsPipeline graphicsPipelineInstance;

	MpgxResult mpgxResult = createGraphicsPipeline(
		framebuffer,
		name,
		state,
		onGlBind,
		onGlUniformsSet,
		onGlResize,
		onGlDestroy,
		handle,
		NULL,
		shaders,
		shaderCount,
		&graphicsPipelineInstance);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		onGlDestroy(window, handle);
		return mpgxResult;
	}

	GLuint glHandle = graphicsPipelineInstance->gl.glHandle;

	GLint mvpLocation, atlasLocation, colorLocation;

	bool result = getGlUniformLocation(
		glHandle,
		"u_MVP",
		&mvpLocation);
	result &= getGlUniformLocation(
		glHandle,
		"u_Atlas",
		&atlasLocation);
	result &= getGlUniformLocation(
		glHandle,
		"u_Color",
		&colorLocation);

	if (!result)
	{
		destroyGraphicsPipeline(graphicsPipelineInstance);
		return BAD_SHADER_CODE_MPGX_RESULT;
	}

	assertOpenGL();

	handle->gl.mvpLocation = mvpLocation;
	handle->gl.atlasLocation = atlasLocation;
	handle->gl.colorLocation = colorLocation;

	*graphicsPipeline = graphicsPipelineInstance;
	return SUCCESS_MPGX_RESULT;
}
#endif

MpgxResult createTextPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Sampler sampler,
	const GraphicsPipelineState* state,
	bool useScissors,
	size_t capacity,
	GraphicsPipeline* textPipeline)
{
	assert(framebuffer);
	assert(vertexShader);
	assert(fragmentShader);
	assert(sampler);
	assert(capacity > 0);
	assert(textPipeline);
	assert(vertexShader->base.type == VERTEX_SHADER_TYPE);
	assert(fragmentShader->base.type == FRAGMENT_SHADER_TYPE);
	assert(vertexShader->base.window == framebuffer->base.window);
	assert(fragmentShader->base.window == framebuffer->base.window);
	assert(sampler->base.window == framebuffer->base.window);

	Handle handle = calloc(1, sizeof(Handle_T));

	if (!handle)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	Text* texts = malloc(
		capacity * sizeof(Text));

	if (!texts)
	{
		free(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	handle->base.sampler = sampler;
	handle->base.vpc.mvp = identMat4F;
	handle->base.fpc.color = whiteLinearColor;
	handle->base.texts = texts;
	handle->base.textCapacity = capacity;
	handle->base.textCount = 0;
	handle->base.vertexBuffer = NULL;
	handle->base.vertexCapacity = 0;
	handle->base.indexBuffer = NULL;

#ifndef NDEBUG
	const char* name = TEXT_PIPELINE_NAME;
#else
	const char* name = NULL;
#endif

	Vec2I framebufferSize =
		framebuffer->base.size;
	Vec4I size = vec4I(0, 0,
		framebufferSize.x,
		framebufferSize.y);

	GraphicsPipelineState defaultState = {
		TRIANGLE_LIST_DRAW_MODE,
		FILL_POLYGON_MODE,
		BACK_CULL_MODE,
		LESS_COMPARE_OPERATOR,
		ALL_COLOR_COMPONENT,
		SOURCE_ALPHA_BLEND_FACTOR,
		ONE_MINUS_SOURCE_ALPHA_BLEND_FACTOR,
		ONE_BLEND_FACTOR,
		ZERO_BLEND_FACTOR,
		ADD_BLEND_OPERATOR,
		ADD_BLEND_OPERATOR,
		true,
		true,
		true,
		false,
		false,
		false,
		true,
		false,
		false,
		DEFAULT_LINE_WIDTH,
		size,
		useScissors ? zeroVec4I : size,
		defaultDepthRange,
		defaultDepthBias,
		defaultBlendColor,
	};

	Shader shaders[2] = {
		vertexShader,
		fragmentShader,
	};

	Window window = framebuffer->base.window;
	GraphicsAPI api = getGraphicsAPI();

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		return createVkPipeline(
			framebuffer,
			window,
			name,
			state ? state : &defaultState,
			handle,
			shaders,
			2,
			textPipeline);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		return createGlPipeline(
			framebuffer,
			window,
			name,
			state ? state : &defaultState,
			handle,
			shaders,
			2,
			textPipeline);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}
}

Sampler getTextPipelineSampler(
	GraphicsPipeline textPipeline)
{
	assert(textPipeline);
	assert(strcmp(textPipeline->base.name,
		TEXT_PIPELINE_NAME) == 0);
	Handle handle = textPipeline->base.handle;
	return handle->base.sampler;
}

Mat4F getTextPipelineMVP(
	GraphicsPipeline textPipeline)
{
	assert(textPipeline);
	assert(strcmp(textPipeline->base.name,
		TEXT_PIPELINE_NAME) == 0);
	Handle handle = textPipeline->base.handle;
	return handle->base.vpc.mvp;
}
void setTextPipelineMVP(
	GraphicsPipeline textPipeline,
	Mat4F mvp)
{
	assert(textPipeline);
	assert(strcmp(textPipeline->base.name,
		TEXT_PIPELINE_NAME) == 0);
	Handle handle = textPipeline->base.handle;
	handle->base.vpc.mvp = mvp;
}

LinearColor getTextPipelineColor(
	GraphicsPipeline textPipeline)
{
	assert(textPipeline);
	assert(strcmp(textPipeline->base.name,
		TEXT_PIPELINE_NAME) == 0);
	Handle handle = textPipeline->base.handle;
	return handle->base.fpc.color;
}
void setTextPipelineColor(
	GraphicsPipeline textPipeline,
	LinearColor color)
{
	assert(textPipeline);
	assert(strcmp(textPipeline->base.name,
		TEXT_PIPELINE_NAME) == 0);
	Handle handle = textPipeline->base.handle;
	handle->base.fpc.color = color;
}
