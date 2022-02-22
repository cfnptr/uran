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
#include "mpgx/_source/graphics_pipeline.h"
#include "mpgx/_source/image.h"
#include "mpgx/_source/sampler.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#include "cmmt/common.h"
#include <assert.h>

// TODO: possibly bake in separated text pipeline thread
// TODO: improve construction steps (like in rederer)

struct Font_T
{
	uint8_t* data;
	FT_Face face;
};

struct Text_T
{
	Font font;
	GraphicsPipeline pipeline;
	uint32_t* data;
	size_t dataCapacity;
	size_t dataLength;
	Image texture;
	GraphicsMesh mesh;
	Vec2F textSize;
	uint32_t fontSize;
	AlignmentType alignment;
	bool isConstant;
#if MPGX_SUPPORT_VULKAN
	uint8_t _alignment[2];
	VkDescriptorPool descriptorPool;
	VkDescriptorSet* descriptorSets;
#endif
};

typedef struct Glyph
{
	Vec4F position;
	Vec4F texCoords;
	float advance;
	uint32_t value;
	bool isVisible;
} Glyph;

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
	Window window;
	Image texture;
	Sampler sampler;
	VertexPushConstants vpc;
	FragmentPushConstants fpc;
	Text* texts;
	size_t textCapacity;
	size_t textCount;
} BaseHandle;
#if MPGX_SUPPORT_VULKAN
typedef struct VkHandle
{
	Window window;
	Image texture;
	Sampler sampler;
	VertexPushConstants vpc;
	FragmentPushConstants fpc;
	Text* texts;
	size_t textCapacity;
	size_t textCount;
	VkDescriptorSetLayout descriptorSetLayout;
	uint32_t bufferCount;
} VkHandle;
#endif
#if MPGX_SUPPORT_OPENGL
typedef struct GlHandle
{
	Window window;
	Image texture;
	Sampler sampler;
	VertexPushConstants vpc;
	FragmentPushConstants fpc;
	Text* texts;
	size_t textCapacity;
	size_t textCount;
	GLint mvpLocation;
	GLint colorLocation;
	GLint textureLocation;
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

bool initializeText()
{
	if (textInitialized)
		return false;

	if (FT_Init_FreeType(&ftLibrary) == 0)
		return false;

	textInitialized = true;
	return true;
}
void terminateText()
{
	if (!textInitialized)
		return;

	if (FT_Done_FreeType(ftLibrary) != 0)
		abort();

	ftLibrary = NULL;
}
bool isTextInitialized()
{
	return textInitialized;
}

bool createStringUTF8(
	const uint32_t* data,
	size_t dataLength,
	char** _array,
	size_t* _arrayLength)
{
	assert(data);
	assert(dataLength > 0);
	assert(_array);
	assert(_arrayLength);

	size_t arrayLength = 0;

	for (size_t i = 0; i < dataLength; i++)
	{
		uint32_t value = data[i];

		if (value < 128)
			arrayLength += 1;
		else if (value < 2048)
			arrayLength += 2;
		else if (value < 65536)
			arrayLength += 3;
		else if (value < 2097152)
			arrayLength += 4;
		else
			return false;
	}

	char* array = malloc(
		(arrayLength + 1) * sizeof(char));

	if (!array)
		return false;

	for (size_t i = 0, j = 0; i < dataLength; i++)
	{
		uint32_t value = data[i];

		if (value < 128)
		{
			array[j] = (char)value;
			j += 1;
		}
		else if (value < 2048)
		{
			array[j] = (char)(((value >> 6) | 0b11000000) & 0b11011111);
			array[j + 1] = (char)((value | 0b10000000) & 0b10111111);
			j += 2;
		}
		else if (value < 65536)
		{
			array[j] = (char)(((value >> 12) | 0b11100000) & 0b11101111);
			array[j + 1] = (char)(((value >> 6) | 0b10000000) & 0b10111111);
			array[j + 2] = (char)((value | 0b10000000) & 0b10111111);
			j += 3;
		}
		else
		{
			array[j] = (char)(((value >> 18) | 0b11110000) & 0b11110111);
			array[j + 1] = (char)(((value >> 12) | 0b10000000) & 0b10111111);
			array[j + 2] = (char)(((value >> 6) | 0b10000000) & 0b10111111);
			array[j + 3] = (char)((value | 0b10000000) & 0b10111111);
			j += 4;
		}
	}

	array[arrayLength] = '\0';

	*_array = array;
	*_arrayLength = arrayLength;
	return true;
}
void destroyStringUTF8(char* array)
{
	free(array);
}

bool validateStringUTF8(
	const char* array,
	size_t arrayLength)
{
	assert(array);
	assert(arrayLength > 0);

	for (size_t i = 0; i < arrayLength; i++)
	{
		char value = array[i];

		if (!((value & 0b10000000) == 0) ||

			!(i + 1 < arrayLength &&
			(value & 0b11100000) == 0b11000000 &&
			(array[i + 1] & 0b11000000) == 0b10000000) ||

			!(i + 2 < arrayLength &&
			(value & 0b11110000) == 0b11100000 &&
			(array[i + 1] & 0b11000000) == 0b10000000 &&
			(array[i + 2] & 0b11000000) == 0b10000000) ||

			!(i + 3 < arrayLength &&
			(value & 0b11111000) == 0b11110000 &&
			(array[i + 1] & 0b11000000) == 0b10000000 &&
			(array[i + 2] & 0b11000000) == 0b10000000 &&
			(array[i + 3] & 0b11000000) == 0b10000000))
		{
			return false;
		}
	}

	return true;
}

bool createStringUTF32(
	const char* data,
	size_t dataLength,
	uint32_t** _array,
	size_t* _arrayLength)
{
	assert(data);
	assert(dataLength > 0);
	assert(_array);
	assert(_arrayLength);

	size_t arrayLength = 0;

	for (size_t i = 0; i < dataLength;)
	{
		char value = data[i];

		if ((value & 0b10000000) == 0)
		{
			i += 1;
		}
		else if (i + 1 < dataLength &&
			(value & 0b11100000) == 0b11000000 &&
			(data[i + 1] & 0b11000000) == 0b10000000)
		{
			i += 2;
		}
		else if (i + 2 < dataLength &&
			(value & 0b11110000) == 0b11100000 &&
			(data[i + 1] & 0b11000000) == 0b10000000 &&
			(data[i + 2] & 0b11000000) == 0b10000000)
		{
			i += 3;
		}
		else if (i + 3 < dataLength &&
			(value & 0b11111000) == 0b11110000 &&
			(data[i + 1] & 0b11000000) == 0b10000000 &&
			(data[i + 2] & 0b11000000) == 0b10000000 &&
			(data[i + 3] & 0b11000000) == 0b10000000)
		{
			i += 4;
		}
		else
		{
			return false;
		}

		arrayLength++;
	}

	uint32_t* array = malloc(
		(arrayLength + 1) * sizeof(uint32_t));

	if (!array)
		return false;

	for (size_t i = 0, j = 0; i < dataLength; j++)
	{
		char value = data[i];

		if ((value & 0b10000000) == 0)
		{
			array[j] = (uint32_t)data[i];
			i += 1;
		}
		else if ((value & 0b11100000) == 0b11000000)
		{
			array[j] = (uint32_t)(data[i] & 0b00011111) << 6 |
				(uint32_t)(data[i + 1] & 0b00111111);
			i += 2;
		}
		else if ((value & 0b11110000) == 0b11100000)
		{
			array[j] = (uint32_t)(data[i] & 0b00001111) << 12 |
				(uint32_t)(data[i + 1] & 0b00111111) << 6 |
				(uint32_t)(data[i + 2] & 0b00111111);
			i += 3;
		}
		else
		{
			array[j] = (uint32_t)(data[i] & 0b00000111) << 18 |
				(uint32_t)(data[i + 1] & 0b00111111) << 12 |
				(uint32_t)(data[i + 2] & 0b00111111) << 6 |
				(uint32_t)(data[i + 3] & 0b00111111);
			i += 4;
		}
	}

	array[arrayLength] = 0;

	*_array = array;
	*_arrayLength = arrayLength;
	return true;
}
void destroyStringUTF32(uint32_t* array)
{
	free(array);
}

bool validateStringUTF32(
	const uint32_t* array,
	size_t arrayLength)
{
	assert(array);
	assert(arrayLength > 0);

	for (size_t i = 0; i < arrayLength; i++)
	{
		if (array[i] >= 2097152)
			return false;
	}

	return true;
}

MpgxResult createFont(
	const void* _data,
	size_t size,
	Font* font)
{
	assert(_data);
	assert(size > 0);
	assert(font);

	Font fontInstance = malloc(sizeof(Font_T));

	if (!fontInstance)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	uint8_t* data = malloc(
		size * sizeof(uint8_t));

	if (!data)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	memcpy(data, _data, size);

	FT_Face face;

	FT_Error result = FT_New_Memory_Face(
		ftLibrary,
		data,
		(FT_Long)size,
		0,
		&face);

	if (result != 0)
	{
		free(data);
		free(font);
		return UNKNOWN_ERROR_MPGX_RESULT; // TODO: handle freetype error
	}

	result = FT_Select_Charmap(
		face,
		FT_ENCODING_UNICODE);

	if (result != 0)
	{
		FT_Done_Face(face);
		free(data);
		free(font);
		return UNKNOWN_ERROR_MPGX_RESULT;
	}

	fontInstance->data = data;
	fontInstance->face = face;

	*font = fontInstance;
	return SUCCESS_MPGX_RESULT;
}
MpgxResult createFontFromFile(
	const void* filePath,
	Font* font)
{
	assert(filePath);
	assert(font);

	Font fontInstance = malloc(sizeof(Font_T));

	if (!fontInstance)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	FT_Face face;

	FT_Error result = FT_New_Face(
		ftLibrary,
		filePath,
		0,
		&face);

	if (result != 0)
	{
		free(font);
		return UNKNOWN_ERROR_MPGX_RESULT; // TODO: handle freetype error
	}

	result = FT_Select_Charmap(
		face,
		FT_ENCODING_UNICODE);

	if (result != 0)
	{
		FT_Done_Face(face);
		free(font);
		return UNKNOWN_ERROR_MPGX_RESULT;
	}

	fontInstance->data = NULL;
	fontInstance->face = face;

	*font = fontInstance;
	return SUCCESS_MPGX_RESULT;
}
void destroyFont(Font font)
{
	assert(textInitialized);

	if (!font)
		return;

	FT_Done_Face(font->face);
	free(font->data);
	free(font);
}

static int compareGlyph(
	const void* a,
	const void* b)
{
	// NOTE: a and b should not be NULL!
	// Skipping assertion for debug build speed.

	if (((Glyph*)a)->value < ((Glyph*)b)->value)
		return -1;
	if (((Glyph*)a)->value == ((Glyph*)b)->value)
		return 0;
	if (((Glyph*)a)->value > ((Glyph*)b)->value)
		return 1;

	abort();
}
inline static bool createTextGlyphs( // TODO: use instead of bools MpgxResults
	const uint32_t* array,
	size_t arrayLength,
	Glyph** _glyphs,
	size_t* _glyphCount)
{
	assert(array);
	assert(arrayLength > 0);
	assert(_glyphs);
	assert(_glyphCount > 0);

	Glyph* glyphs = malloc(
		arrayLength * sizeof(Glyph));

	if (!glyphs)
		return false;

	size_t glyphCount = 0;

	for (size_t i = 0; i < arrayLength; i++)
	{
		uint32_t value = array[i];

		if (value == '\n')
			continue;
		else if (value == '\t')
			value = ' ';

		Glyph searchGlyph;
		searchGlyph.value = value;

		Glyph* glyph = bsearch(
			&searchGlyph,
			glyphs,
			glyphCount,
			sizeof(Glyph),
			compareGlyph);

		if (!glyph)
		{
			glyphs[glyphCount].value = value;
			glyphCount++;

			qsort(
				glyphs,
				glyphCount,
				sizeof(Glyph),
				compareGlyph);
		}
	}

	if (glyphCount == 0)
	{
		free(glyphs);
		return false;
	}

	*_glyphs = glyphs;
	*_glyphCount = glyphCount;
	return true;
}
inline static bool createTextPixels(
	FT_Face face,
	uint32_t fontSize,
	Glyph* glyphs,
	size_t glyphCount,
	uint32_t textPixelLength,
	uint8_t** _pixels,
	size_t* _pixelCount,
	uint32_t* _pixelLength)
{
	assert(face);
	assert(fontSize > 0);
	assert(glyphs);
	assert(glyphCount > 0);
	assert(textPixelLength > 0);
	assert(_pixels);
	assert(_pixelCount);
	assert(_pixelLength);

	uint32_t glyphLength = (uint32_t)ceilf(sqrtf((float)glyphCount));
	uint32_t pixelLength = glyphLength * fontSize;
	size_t pixelCount = (size_t)pixelLength * pixelLength;

	uint8_t* pixels = calloc(
		pixelCount,
		sizeof(uint8_t));

	if (!pixels)
		return false;

	if (textPixelLength < pixelLength)
		textPixelLength = pixelLength;

	for (size_t i = 0; i < glyphCount; i++)
	{
		Glyph glyph;
		glyph.value = glyphs[i].value;

		FT_UInt charIndex = FT_Get_Char_Index(
			face,
			glyph.value);
		FT_Error result = FT_Load_Glyph(
			face,
			charIndex,
			FT_LOAD_RENDER);

		if (result != 0)
		{
			free(pixels);
			return false;
		}

		FT_GlyphSlot glyphSlot = face->glyph;
		uint8_t* bitmap = glyphSlot->bitmap.buffer;

		uint32_t pixelPosY = (uint32_t)(i / glyphLength);
		uint32_t pixelPosX = (uint32_t)(i - pixelPosY * glyphLength);

		pixelPosX *= fontSize;
		pixelPosY *= fontSize;

		uint32_t glyphWidth = glyphSlot->bitmap.width;
		uint32_t glyphHeight = glyphSlot->bitmap.rows;

		if (glyphWidth * glyphHeight == 0)
		{
			glyph.position = zeroVec4F;
			glyph.texCoords = zeroVec4F;
			glyph.isVisible = false;
		}
		else
		{
			glyph.position.x = (float)glyphSlot->bitmap_left / (float)fontSize;
			glyph.position.y = ((float)glyphSlot->bitmap_top - (float)glyphHeight) / (float)fontSize;
			glyph.position.z = glyph.position.x + (float)glyphWidth / (float)fontSize;
			glyph.position.w = glyph.position.y + (float)glyphHeight /(float)fontSize;
			glyph.texCoords.x = (float)pixelPosX / (float)textPixelLength;
			glyph.texCoords.y = (float)pixelPosY / (float)textPixelLength;
			glyph.texCoords.z = glyph.texCoords.x + (float)glyphWidth / (float)textPixelLength;
			glyph.texCoords.w = glyph.texCoords.y + (float)glyphHeight / (float)textPixelLength;
			glyph.isVisible = true;

			for (size_t y = 0; y < glyphHeight; y++)
			{
				for (size_t x = 0; x < glyphWidth; x++)
				{
					pixels[(y + pixelPosY) * pixelLength + (x + pixelPosX)] =
						bitmap[y * glyphWidth + x];
				}
			}
		}

		glyph.advance = ((float)glyphSlot->advance.x / 64.0f) / (float)fontSize;
		glyphs[i] = glyph;
	}

	*_pixels = pixels;
	*_pixelCount = pixelCount;
	*_pixelLength = pixelLength;
	return true;
}
inline static bool createTextVertices(
	const uint32_t* array,
	size_t arrayLength,
	const Glyph* glyphs,
	size_t glyphCount,
	float newLineAdvance,
	AlignmentType alignment,
	float** _vertices,
	size_t* _vertexCount,
	Vec2F* _textSize)
{
	assert(array);
	assert(arrayLength > 0);
	assert(glyphs);
	assert(glyphCount > 0);
	assert(_vertices);
	assert(_vertexCount);
	assert(_textSize);

	// TODO: use mapBuffer here
	size_t vertexCount = arrayLength * 16;
	float* vertices = malloc(vertexCount * sizeof(float));

	if (!vertices)
		return false;

	Vec2F vertexOffset = vec2F(
		0.0f, -newLineAdvance * 0.5f);
	Vec2F textSize = zeroVec2F;

	size_t vertexIndex = 0;
	size_t lastNewLineIndex = 0;

	float offset;

	for (size_t i = 0; i < arrayLength; i++)
	{
		uint32_t value = array[i];

		if (value == '\n')
		{
			switch (alignment)
			{
			default:
				abort();
			case CENTER_ALIGNMENT_TYPE:
				offset = vertexOffset.x * -0.5f;

				for (size_t j = lastNewLineIndex; j < vertexIndex; j += 16)
				{
					vertices[j + 0] += offset;
					vertices[j + 4] += offset;
					vertices[j + 8] += offset;
					vertices[j + 12] += offset;
				}
				break;
			case LEFT_ALIGNMENT_TYPE:
				break;
			case RIGHT_ALIGNMENT_TYPE:
				offset = -vertexOffset.x;

				for (size_t j = lastNewLineIndex; j < vertexIndex; j += 16)
				{
					vertices[j + 0] += offset;
					vertices[j + 4] += offset;
					vertices[j + 8] += offset;
					vertices[j + 12] += offset;
				}
				break;
			case BOTTOM_ALIGNMENT_TYPE:
				offset = vertexOffset.x * -0.5f;

				for (size_t j = lastNewLineIndex; j < vertexIndex; j += 16)
				{
					vertices[j + 0] += offset;
					vertices[j + 4] += offset;
					vertices[j + 8] += offset;
					vertices[j + 12] += offset;
				}
				break;
			case TOP_ALIGNMENT_TYPE:
				offset = vertexOffset.x * -0.5f;

				for (size_t j = lastNewLineIndex; j < vertexIndex; j += 16)
				{
					vertices[j + 0] += offset;
					vertices[j + 4] += offset;
					vertices[j + 8] += offset;
					vertices[j + 12] += offset;
				}
				break;
			case LEFT_BOTTOM_ALIGNMENT_TYPE:
			case LEFT_TOP_ALIGNMENT_TYPE:
				break;
			case RIGHT_BOTTOM_ALIGNMENT_TYPE:
				offset = -vertexOffset.x;

				for (size_t j = lastNewLineIndex; j < vertexIndex; j += 16)
				{
					vertices[j + 0] += offset;
					vertices[j + 4] += offset;
					vertices[j + 8] += offset;
					vertices[j + 12] += offset;
				}
				break;
			case RIGHT_TOP_ALIGNMENT_TYPE:
				offset = -vertexOffset.x;

				for (size_t j = lastNewLineIndex; j < vertexIndex; j += 16)
				{
					vertices[j + 0] += offset;
					vertices[j + 4] += offset;
					vertices[j + 8] += offset;
					vertices[j + 12] += offset;
				}
				break;
			}

			lastNewLineIndex = vertexIndex;

			if (textSize.x < vertexOffset.x)
				textSize.x = vertexOffset.x;

			vertexOffset.y -= newLineAdvance;
			vertexOffset.x = 0.0f;
			continue;
		}
		else if (value == '\t')
		{
			Glyph searchGlyph;
			searchGlyph.value = ' ';

			Glyph* glyph = bsearch(
				&searchGlyph,
				glyphs,
				glyphCount,
				sizeof(Glyph),
				compareGlyph);

			if (!glyph)
			{
				free(vertices);
				return false;
			}

			vertexOffset.x += glyph->advance * 4;
			continue;
		}

		Glyph searchGlyph;
		searchGlyph.value = value;

		Glyph* glyph = bsearch(
			&searchGlyph,
			glyphs,
			glyphCount,
			sizeof(Glyph),
			compareGlyph);

		if (!glyph)
		{
			free(vertices);
			return false;
		}

		if (glyph->isVisible)
		{
			Vec4F position = vec4F(
				vertexOffset.x + glyph->position.x,
				vertexOffset.y + glyph->position.y,
				vertexOffset.x + glyph->position.z,
				vertexOffset.y + glyph->position.w);
			Vec4F texCoords = glyph->texCoords;

			vertices[vertexIndex + 0] = position.x;
			vertices[vertexIndex + 1] = position.y;
			vertices[vertexIndex + 2] = texCoords.x;
			vertices[vertexIndex + 3] = texCoords.w;
			vertices[vertexIndex + 4] = position.x;
			vertices[vertexIndex + 5] = position.w;
			vertices[vertexIndex + 6] = texCoords.x;
			vertices[vertexIndex + 7] = texCoords.y;
			vertices[vertexIndex + 8] = position.z;
			vertices[vertexIndex + 9] = position.w;
			vertices[vertexIndex + 10] = texCoords.z;
			vertices[vertexIndex + 11] = texCoords.y;
			vertices[vertexIndex + 12] = position.z;
			vertices[vertexIndex + 13] = position.y;
			vertices[vertexIndex + 14] = texCoords.z;
			vertices[vertexIndex + 15] = texCoords.w;

			vertexIndex += 16;
		}

		vertexOffset.x += glyph->advance;
	}

	if (vertexIndex == 0)
	{
		free(vertices);
		return false;
	}

	if (textSize.x < vertexOffset.x)
		textSize.x = vertexOffset.x;
	textSize.y = -vertexOffset.y;

	switch (alignment)
	{
	default:
		abort();
	case CENTER_ALIGNMENT_TYPE:
		offset = vertexOffset.x * -0.5f;

		for (size_t i = lastNewLineIndex; i < vertexIndex; i += 16)
		{
			vertices[i + 0] += offset;
			vertices[i + 4] += offset;
			vertices[i + 8] += offset;
			vertices[i + 12] += offset;
		}

		offset = textSize.y * 0.5f;

		for (size_t i = 0; i < vertexIndex; i += 16)
		{
			vertices[i + 1] += offset;
			vertices[i + 5] += offset;
			vertices[i + 9] += offset;
			vertices[i + 13] += offset;
		}
		break;
	case LEFT_ALIGNMENT_TYPE:
		offset = textSize.y * 0.5f;

		for (size_t i = 0; i < vertexIndex; i += 16)
		{
			vertices[i + 1] += offset;
			vertices[i + 5] += offset;
			vertices[i + 9] += offset;
			vertices[i + 13] += offset;
		}
		break;
	case RIGHT_ALIGNMENT_TYPE:
		offset = -vertexOffset.x;

		for (size_t i = lastNewLineIndex; i < vertexIndex; i += 16)
		{
			vertices[i + 0] += offset;
			vertices[i + 4] += offset;
			vertices[i + 8] += offset;
			vertices[i + 12] += offset;
		}

		offset = textSize.y * 0.5f;

		for (size_t i = 0; i < vertexIndex; i += 16)
		{
			vertices[i + 1] += offset;
			vertices[i + 5] += offset;
			vertices[i + 9] += offset;
			vertices[i + 13] += offset;
		}
		break;
	case BOTTOM_ALIGNMENT_TYPE:
		offset = vertexOffset.x * -0.5f;

		for (size_t i = lastNewLineIndex; i < vertexIndex; i += 16)
		{
			vertices[i + 0] += offset;
			vertices[i + 4] += offset;
			vertices[i + 8] += offset;
			vertices[i + 12] += offset;
		}

		offset = textSize.y;

		for (size_t i = 0; i < vertexIndex; i += 16)
		{
			vertices[i + 1] += offset;
			vertices[i + 5] += offset;
			vertices[i + 9] += offset;
			vertices[i + 13] += offset;
		}
		break;
	case TOP_ALIGNMENT_TYPE:
		offset = vertexOffset.x * -0.5f;

		for (size_t i = lastNewLineIndex; i < vertexIndex; i += 16)
		{
			vertices[i + 0] += offset;
			vertices[i + 4] += offset;
			vertices[i + 8] += offset;
			vertices[i + 12] += offset;
		}
		break;
	case LEFT_BOTTOM_ALIGNMENT_TYPE:
		offset = textSize.y;

		for (size_t i = 0; i < vertexIndex; i += 16)
		{
			vertices[i + 1] += offset;
			vertices[i + 5] += offset;
			vertices[i + 9] += offset;
			vertices[i + 13] += offset;
		}
		break;
	case LEFT_TOP_ALIGNMENT_TYPE:
		break;
	case RIGHT_BOTTOM_ALIGNMENT_TYPE:
		offset = -vertexOffset.x;

		for (size_t i = lastNewLineIndex; i < vertexIndex; i += 16)
		{
			vertices[i + 0] += offset;
			vertices[i + 4] += offset;
			vertices[i + 8] += offset;
			vertices[i + 12] += offset;
		}

		offset = textSize.y;

		for (size_t i = 0; i < vertexIndex; i += 16)
		{
			vertices[i + 1] += offset;
			vertices[i + 5] += offset;
			vertices[i + 9] += offset;
			vertices[i + 13] += offset;
		}
		break;
	case RIGHT_TOP_ALIGNMENT_TYPE:
		offset = -vertexOffset.x;

		for (size_t i = lastNewLineIndex; i < vertexIndex; i += 16)
		{
			vertices[i + 0] += offset;
			vertices[i + 4] += offset;
			vertices[i + 8] += offset;
			vertices[i + 12] += offset;
		}
		break;
	}

	*_vertices = vertices;
	*_vertexCount = vertexIndex;
	*_textSize = textSize;
	return true;
}
inline static bool createTextIndices(
	size_t vertexCount,
	uint32_t** _indices,
	size_t* _indexCount)
{
	assert(vertexCount > 0);
	assert(_indices);
	assert(_indexCount > 0);

	size_t indexCount = (vertexCount / 16) * 6;

	uint32_t* indices = malloc(
		indexCount * sizeof(uint32_t));

	if (!indices)
		return false;

	for (size_t i = 0, j = 0; i < indexCount; i += 6, j += 4)
	{
		indices[i + 0] = (uint32_t)j + 0;
		indices[i + 1] = (uint32_t)j + 1;
		indices[i + 2] = (uint32_t)j + 2;
		indices[i + 3] = (uint32_t)j + 0;
		indices[i + 4] = (uint32_t)j + 2;
		indices[i + 5] = (uint32_t)j + 3;
	}

	*_indices = indices;
	*_indexCount = indexCount;
	return true;
}

#if MPGX_SUPPORT_VULKAN
inline static MpgxResult createVkDescriptorPoolInstance(
	VkDevice device,
	uint32_t bufferCount,
	VkDescriptorPool* descriptorPool)
{
	assert(device);
	assert(bufferCount > 0);
	assert(descriptorPool);

	VkDescriptorPoolSize descriptorPoolSizes[1] = {
		{
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			bufferCount * 2, // TODO: why * 2 ?
		},
	};
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		NULL,
		0,
		bufferCount,
		1,
		descriptorPoolSizes
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
inline static MpgxResult createVkDescriptorSetArray(
	VkDevice device,
	VkDescriptorSetLayout descriptorSetLayout,
	VkDescriptorPool descriptorPool,
	uint32_t bufferCount,
	VkSampler sampler,
	VkImageView imageView,
	VkDescriptorSet** descriptorSets)
{
	assert(device);
	assert(descriptorSetLayout);
	assert(descriptorPool);
	assert(bufferCount > 0);
	assert(sampler);
	assert(imageView);
	assert(descriptorSets);

	VkDescriptorSet* descriptorSetArray;

	// TODO: possibly allocate only one descriptor set?
	MpgxResult mpgxResult = allocateVkDescriptorSets(
		device,
		descriptorSetLayout,
		descriptorPool,
		bufferCount,
		&descriptorSetArray);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	for (uint32_t i = 0; i < bufferCount; i++)
	{
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
				descriptorSetArray[i],
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
	}

	*descriptorSets = descriptorSetArray;
	return SUCCESS_MPGX_RESULT;
}
#endif

MpgxResult createText32(
	GraphicsPipeline textPipeline,
	Font font,
	uint32_t fontSize,
	AlignmentType alignment,
	const uint32_t* _data,
	size_t dataLength,
	bool isConstant,
	Text* text)
{
	assert(textPipeline);
	assert(font);
	assert(fontSize > 0);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(_data);
	assert(dataLength > 0);
	assert(text);

	assert(strcmp(textPipeline->base.name,
		TEXT_PIPELINE_NAME) == 0);

	Text textInstance = malloc(sizeof(Text_T));

	if (!textInstance)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	uint32_t* data = malloc(
		dataLength * sizeof(uint32_t));

	if (!data)
	{
		free(text);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	memcpy(data, _data,
		dataLength * sizeof(uint32_t));

	Glyph* glyphs;
	size_t glyphCount;

	bool result = createTextGlyphs(
		_data,
		dataLength,
		&glyphs,
		&glyphCount);

	if (!result)
	{
		free(data);
		free(text);
		return UNKNOWN_ERROR_MPGX_RESULT; // TODO: handle bad text data
	}

	FT_Face face = font->face;

	FT_Error ftResult = FT_Set_Pixel_Sizes(
		face,
		0,
		(FT_UInt)fontSize);

	if (ftResult != 0)
	{
		free(glyphs);
		free(data);
		free(text);
		return UNKNOWN_ERROR_MPGX_RESULT;
	}

	float newLineAdvance =
		((float)face->size->metrics.height / 64.0f) /
		(float)fontSize;

	uint8_t* pixels;
	size_t pixelCount;
	uint32_t pixelLength;

	result = createTextPixels(
		face,
		fontSize,
		glyphs,
		glyphCount,
		0,
		&pixels,
		&pixelCount,
		&pixelLength);

	if (!result)
	{
		free(glyphs);
		free(data);
		free(text);
		return UNKNOWN_ERROR_MPGX_RESULT;
	}

	Window window = textPipeline->base.framebuffer->base.window;

	Image texture;

	MpgxResult mpgxResult = createImage(
		window,
		SAMPLED_IMAGE_TYPE,
		IMAGE_2D,
		R8_UNORM_IMAGE_FORMAT,
		(const void**)&pixels,
		vec3I(pixelLength, pixelLength, 1),
		1,
		isConstant,
		&texture);

	free(pixels);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		free(glyphs);
		free(data);
		free(text);
		return mpgxResult;
	}

	float* vertices;
	size_t vertexCount;
	Vec2F textSize;

	result = createTextVertices(
		data,
		dataLength,
		glyphs,
		glyphCount,
		newLineAdvance,
		alignment,
		&vertices,
		&vertexCount,
		&textSize);

	free(glyphs);

	if (!result)
	{
		destroyImage(texture);
		free(data);
		free(text);
		return UNKNOWN_ERROR_MPGX_RESULT;
	}

	Buffer vertexBuffer;

	mpgxResult = createBuffer(
		window,
		VERTEX_BUFFER_TYPE,
		isConstant ?
			GPU_ONLY_BUFFER_USAGE :
			CPU_TO_GPU_BUFFER_USAGE,
		vertices,
		vertexCount * sizeof(float),
		&vertexBuffer);

	free(vertices);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyImage(texture);
		free(data);
		free(text);
		return mpgxResult;
	}

	uint32_t* indices;
	size_t indexCount;

	result = createTextIndices(
		vertexCount,
		&indices,
		&indexCount);

	if (!result)
	{
		destroyBuffer(vertexBuffer);
		destroyImage(texture);
		free(data);
		free(text);
		return UNKNOWN_ERROR_MPGX_RESULT;
	}

	Buffer indexBuffer;

	mpgxResult = createBuffer(
		window,
		INDEX_BUFFER_TYPE,
		isConstant ?
			GPU_ONLY_BUFFER_USAGE :
			CPU_TO_GPU_BUFFER_USAGE,
		indices,
		indexCount * sizeof(uint32_t),
		&indexBuffer);

	free(indices);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyBuffer(vertexBuffer);
		destroyImage(texture);
		free(data);
		free(text);
		return mpgxResult;
	}

	GraphicsMesh mesh;

	mpgxResult = createGraphicsMesh(
		window,
		UINT32_INDEX_TYPE,
		indexCount,
		0,
		vertexBuffer,
		indexBuffer,
		&mesh);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyBuffer(indexBuffer);
		destroyBuffer(vertexBuffer);
		destroyImage(texture);
		free(data);
		free(text);
		return mpgxResult;
	}

	GraphicsAPI api = getGraphicsAPI();

#if MPGX_SUPPORT_VULKAN
	if (api == VULKAN_GRAPHICS_API)
	{
		VkWindow vkWindow = getVkWindow(window);
		VkDevice device = vkWindow->device;

		Handle handle = textPipeline->vk.handle;
		uint8_t bufferCount = handle->vk.bufferCount;

		VkDescriptorPool descriptorPool;

		mpgxResult = createVkDescriptorPoolInstance(
			device,
			bufferCount,
			&descriptorPool);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			// TODO: destroy transform, add it to the text handle
			destroyGraphicsMesh(mesh);
			destroyImage(texture);
			free(data);
			free(text);
			return mpgxResult;
		}

		VkDescriptorSet* descriptorSets;

		mpgxResult = createVkDescriptorSetArray(
			device,
			handle->vk.descriptorSetLayout,
			descriptorPool,
			bufferCount,
			handle->vk.sampler->vk.handle,
			texture->vk.imageView,
			&descriptorSets);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			vkDestroyDescriptorPool(
				device,
				descriptorPool,
				NULL);
			// TODO: destroy transform, add it to the text handle
			destroyGraphicsMesh(mesh);
			destroyImage(texture);
			free(data);
			free(text);
			return mpgxResult;
		}

		textInstance->descriptorPool = descriptorPool;
		textInstance->descriptorSets = descriptorSets;
	}
	else
	{
		textInstance->descriptorPool = NULL;
		textInstance->descriptorSets = NULL;
	}
#endif

	textInstance->font = font;
	textInstance->pipeline = textPipeline;
	textInstance->data = data;
	textInstance->dataCapacity = dataLength;
	textInstance->dataLength = dataLength;
	textInstance->texture = texture;
	textInstance->mesh = mesh;
	textInstance->textSize = textSize;
	textInstance->fontSize = fontSize;
	textInstance->alignment = alignment;
	textInstance->isConstant = isConstant;

	Handle handle = textPipeline->base.handle;
	size_t count = handle->base.textCount;

	if (count == handle->base.textCapacity)
	{
		size_t capacity = handle->base.textCapacity * 2;

		Text* texts = realloc(
			handle->base.texts,
			capacity * sizeof(Text));

		if (!texts)
		{
#if MPGX_SUPPORT_VULKAN
			if (api == VULKAN_GRAPHICS_API)
			{
				free(textInstance->descriptorSets);

				VkWindow vkWindow = getVkWindow(window);

				vkDestroyDescriptorPool(
					vkWindow->device,
					textInstance->descriptorPool,
					NULL);
			}
#endif
			// TODO: destroy transform, add it to the text handle
			destroyGraphicsMesh(textInstance->mesh);
			destroyImage(textInstance->texture);
			free(textInstance->data);
			free(textInstance);
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		}

		handle->base.texts = texts;
		handle->base.textCapacity = capacity;
	}

	handle->base.texts[count] = textInstance;
	handle->base.textCount = count + 1;

	*text = textInstance;
	return SUCCESS_MPGX_RESULT;
}
MpgxResult createText8(
	GraphicsPipeline textPipeline,
	Font font,
	uint32_t fontSize,
	AlignmentType alignment,
	const char* data,
	size_t dataLength,
	bool isConstant,
	Text* text)
{
	assert(textPipeline);
	assert(font);
	assert(fontSize > 0);
	assert(data);
	assert(dataLength > 0);
	assert(text);

	uint32_t* array;
	size_t arrayLength;

	bool result = createStringUTF32(
		data,
		dataLength,
		&array,
		&arrayLength);

	if (!result)
		return UNKNOWN_ERROR_MPGX_RESULT; // TODO:

	MpgxResult mpgxResult = createText32(
		textPipeline,
		font,
		fontSize,
		alignment,
		array,
		arrayLength,
		isConstant,
		text);

	destroyStringUTF32(array);
	return mpgxResult;
}
void destroyText(Text text)
{
	assert(textInitialized);

	if (!text)
		return;

	Handle handle = text->pipeline->base.handle;
	Text* texts = handle->base.texts;
	size_t textCount = handle->base.textCount;

	for (size_t i = 0; i < textCount; i++)
	{
		if (texts[i] != text)
			continue;

		for (size_t j = i + 1; j < textCount; j++)
			texts[j - 1] = texts[j];

#if MPGX_SUPPORT_VULKAN
		GraphicsPipeline pipeline = text->pipeline;
		GraphicsAPI api = getGraphicsAPI();

		if (api == VULKAN_GRAPHICS_API)
		{
			VkWindow vkWindow = getVkWindow(
				pipeline->vk.framebuffer->vk.window);
			VkDevice device = vkWindow->device;

			VkResult vkResult = vkQueueWaitIdle(
				vkWindow->graphicsQueue);

			if (vkResult != VK_SUCCESS)
				abort();

			free(text->descriptorSets);

			vkDestroyDescriptorPool(
				device,
				text->descriptorPool,
				NULL);
		}
#endif
		// TODO: destroy transform, add it to the text handle
		destroyGraphicsMesh(text->mesh);
		destroyImage(text->texture);

		free(text->data);
		free(text);

		handle->base.textCount--;
		return;
	}

	abort();
}

GraphicsPipeline getTextPipeline(Text text)
{
	assert(text);
	assert(textInitialized);
	return text->pipeline;
}
bool isTextConstant(Text text)
{
	assert(text);
	assert(textInitialized);
	return text->isConstant;
}

Vec2F getTextSize(Text text)
{
	assert(text);
	assert(textInitialized);
	return text->textSize;
}
Vec2F getTextOffset(Text text)
{
	assert(text);
	assert(textInitialized);

	AlignmentType alignment = text->alignment;
	Vec2F textSize = text->textSize;

	switch (alignment)
	{
	default:
		abort();
	case CENTER_ALIGNMENT_TYPE:
		return vec2F(
			-textSize.x * 0.5f,
			textSize.y * 0.5f);
	case LEFT_ALIGNMENT_TYPE:
		return vec2F(
			0.0f,
			textSize.y * 0.5f);
	case RIGHT_ALIGNMENT_TYPE:
		return vec2F(
			-textSize.x,
			textSize.y * 0.5f);
	case BOTTOM_ALIGNMENT_TYPE:
		return vec2F(
			-textSize.x * 0.5f,
			0.0f);
	case TOP_ALIGNMENT_TYPE:
		return vec2F(
			-textSize.x * 0.5f,
			textSize.y);
	case LEFT_BOTTOM_ALIGNMENT_TYPE:
		return vec2F(
			0.0f,
			0.0f);
	case LEFT_TOP_ALIGNMENT_TYPE:
		return vec2F(
			0.0f,
			textSize.y);
	case RIGHT_BOTTOM_ALIGNMENT_TYPE:
		return vec2F(
			-textSize.x,
			0.0f);
	case RIGHT_TOP_ALIGNMENT_TYPE:
		return vec2F(
			-textSize.x,
			textSize.y);
	}
}

// TODO: take into account text alignment
bool getTextCaretAdvance(
	Text text,
	size_t index,
	Vec2F* _advance)
{
	assert(text);
	assert(_advance);
	assert(textInitialized);

	if (index == 0)
	{
		*_advance = zeroVec2F;
		return true;
	}

	FT_Face face = text->font->face;
	uint32_t fontSize = text->fontSize;

	FT_Error ftResult = FT_Set_Pixel_Sizes(
		face,
		0,
		(FT_UInt)fontSize);

	if (ftResult != 0)
		return false;

	float newLineAdvance = ((float)face->size->metrics.height /
		64.0f) / (float)fontSize;

	const uint32_t* data = text->data;
	size_t dataLength = text->dataLength;

	if (index > dataLength)
		index = dataLength;

	Vec2F advance = zeroVec2F;

	for (size_t i = 0; i < index; i++)
	{
		uint32_t value = data[i];

		if (value == '\n')
		{
			advance.y += newLineAdvance;
			advance.x = 0.0f;
			continue;
		}
		else if (value == '\t')
		{
			value = ' ';

			FT_UInt charIndex = FT_Get_Char_Index(
				face,
				value);
			FT_Error result = FT_Load_Glyph(
				face,
				charIndex,
				FT_LOAD_BITMAP_METRICS_ONLY);

			if (result != 0)
				return false;

			advance.x += ((float)face->glyph->advance.x /
				64.0f) / (float)fontSize * 4;
			continue;
		}

		FT_UInt charIndex = FT_Get_Char_Index(
			face,
			value);
		FT_Error result = FT_Load_Glyph(
			face,
			charIndex,
			FT_LOAD_BITMAP_METRICS_ONLY);

		if (result != 0)
			return false;

		advance.x += ((float)face->glyph->advance.x /
			64.0f) / (float)fontSize;
	}

	*_advance = advance;
	return true;
}
bool getTextCaretPosition(
	Text text,
	Vec2F* advance,
	size_t* index)
{
	assert(text);
	assert(advance);
	assert(index);
	assert(textInitialized);

	// TODO:
	abort();
}

Font getTextFont(
	Text text)
{
	assert(text);
	assert(textInitialized);
	return text->font;
}
void setTextFont(
	Text text,
	Font font)
{
	assert(text);
	assert(font);
	assert(!text->isConstant);
	assert(textInitialized);
	text->font = font;
}

uint32_t getTextFontSize(
	Text text)
{
	assert(text);
	assert(textInitialized);
	return text->fontSize;
}
void setTextFontSize(
	Text text,
	uint32_t fontSize)
{
	assert(text);
	assert(!text->isConstant);
	assert(textInitialized);
	text->fontSize = fontSize;
}

AlignmentType getTextAlignment(
	Text text)
{
	assert(text);
	assert(textInitialized);
	return text->alignment;
}
void setTextAlignment(
	Text text,
	AlignmentType alignment)
{
	assert(text);
	assert(!text->isConstant);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(textInitialized);
	text->alignment = alignment;
}

size_t getTextDataLength(Text text)
{
	assert(text);
	assert(textInitialized);
	return text->dataLength;
}
const uint32_t* getTextData(
	Text text)
{
	assert(text);
	assert(textInitialized);
	return text->data;
}

bool setTextData32(
	Text text,
	const uint32_t* _data,
	size_t dataLength,
	bool reuseBuffers)
{
	assert(text);
	assert(_data);
	assert(dataLength > 0);
	assert(!text->isConstant);
	assert(textInitialized);

	if (reuseBuffers)
	{
		if (dataLength > text->dataCapacity)
		{
			uint32_t* data = realloc(
				text->data,
				dataLength * sizeof(uint32_t));

			if (!data)
				return false;

			memcpy(data, _data,
				dataLength * sizeof(uint32_t));

			text->data = data;
			text->dataCapacity = dataLength;
			text->dataLength = dataLength;
			return true;
		}
		else
		{
			memcpy(text->data, _data,
				dataLength * sizeof(uint32_t));
			text->dataLength = dataLength;
			return true;
		}
	}
	else
	{
		uint32_t* data = malloc(
			dataLength * sizeof(uint32_t));

		if (!data)
			return false;

		free(text->data);

		text->data = data;
		text->dataCapacity = dataLength;
		text->dataLength = dataLength;
		return true;
	}
}
bool setTextData8(
	Text text,
	const char* data,
	size_t dataLength,
	bool reuseBuffers)
{
	assert(data);
	assert(dataLength > 0);
	assert(!text->isConstant);

	uint32_t* array;
	size_t arrayLength;

	bool result = createStringUTF32(
		data,
		dataLength,
		&array,
		&arrayLength);

	if (!result)
		return false;

	result = setTextData32(
		text,
		array,
		arrayLength,
		reuseBuffers);

	destroyStringUTF32(array);
	return result;
}

MpgxResult bakeText(
	Text text,
	bool reuseBuffers)
{
	assert(text);
	assert(!text->isConstant);
	assert(textInitialized);

	GraphicsPipeline pipeline = text->pipeline;
	Window window = pipeline->base.framebuffer->base.window;
	GraphicsAPI api = getGraphicsAPI();

	uint32_t* data = text->data;
	size_t dataLength = text->dataLength;

	if (reuseBuffers)
	{
		Glyph* glyphs;
		size_t glyphCount;

		bool result = createTextGlyphs(
			data,
			dataLength,
			&glyphs,
			&glyphCount);

		if (!result)
			return UNKNOWN_ERROR_MPGX_RESULT; // TODO:

		FT_Face face = text->font->face;
		uint32_t fontSize = text->fontSize;

		FT_Error ftResult = FT_Set_Pixel_Sizes(
			face,
			0,
			(FT_UInt)fontSize);

		if (ftResult != 0)
		{
			free(glyphs);
			return UNKNOWN_ERROR_MPGX_RESULT;
		}

		float newLineAdvance =
			((float)face->size->metrics.height / 64.0f) /
			(float)fontSize;

		uint32_t textPixelLength =
			getImageSize(text->texture).x;

		uint8_t* pixels;
		size_t pixelCount;
		uint32_t pixelLength;

		result = createTextPixels(
			face,
			fontSize,
			glyphs,
			glyphCount,
			textPixelLength,
			&pixels,
			&pixelCount,
			&pixelLength);

		if (!result)
		{
			free(glyphs);
			return UNKNOWN_ERROR_MPGX_RESULT;
		}

		Image texture;

#if MPGX_SUPPORT_VULKAN
		VkDescriptorSet* descriptorSets;
#endif

		if (pixelLength > textPixelLength)
		{
			MpgxResult mpgxResult = createImage(
				window,
				SAMPLED_IMAGE_TYPE,
				IMAGE_2D,
				R8_UNORM_IMAGE_FORMAT,
				(const void**)&pixels,
				vec3I(pixelLength, pixelLength, 1),
				1,
				false,
				&texture);

			free(pixels);
			pixels = NULL;

			if (mpgxResult != SUCCESS_MPGX_RESULT)
			{
				free(glyphs);
				return mpgxResult;
			}

#if MPGX_SUPPORT_VULKAN
			if (api == VULKAN_GRAPHICS_API)
			{
				VkWindow vkWindow = getVkWindow(window);
				VkDevice device = vkWindow->device;
				Handle handle = pipeline->vk.handle;

				mpgxResult = createVkDescriptorSetArray(
					device,
					handle->vk.descriptorSetLayout,
					text->descriptorPool,
					handle->vk.bufferCount,
					handle->vk.sampler->vk.handle,
					texture->vk.imageView,
					&descriptorSets);

				if (mpgxResult != SUCCESS_MPGX_RESULT)
				{
					destroyImage(texture);
					free(glyphs);
					return mpgxResult;
				}
			}
#endif
		}
		else
		{
			texture = NULL;
		}

		float* vertices;
		size_t vertexCount;
		Vec2F textSize;

		result = createTextVertices(
			data,
			dataLength,
			glyphs,
			glyphCount,
			newLineAdvance,
			text->alignment,
			&vertices,
			&vertexCount,
			&textSize);

		free(glyphs);

		if (!result)
		{
			destroyImage(texture);
			free(pixels);
			return UNKNOWN_ERROR_MPGX_RESULT;
		}

		Buffer vertexBuffer = NULL;
		Buffer indexBuffer = NULL;

		size_t textVertexBufferSize = getBufferSize(
			getGraphicsMeshVertexBuffer(text->mesh));

		if (vertexCount * sizeof(float) > textVertexBufferSize)
		{
			MpgxResult mpgxResult = createBuffer(
				window,
				VERTEX_BUFFER_TYPE,
				CPU_TO_GPU_BUFFER_USAGE,
				vertices,
				vertexCount * sizeof(float),
				&vertexBuffer);

			free(vertices);

			if (mpgxResult != SUCCESS_MPGX_RESULT)
			{
				destroyImage(texture);
				free(pixels);
				return mpgxResult;
			}

			uint32_t* indices;
			size_t indexCount;

			result = createTextIndices(
				vertexCount,
				&indices,
				&indexCount);

			if (!result)
			{
				destroyBuffer(vertexBuffer);
				destroyImage(texture);
				free(pixels);
				return UNKNOWN_ERROR_MPGX_RESULT;
			}

			mpgxResult = createBuffer(
				window,
				INDEX_BUFFER_TYPE,
				CPU_TO_GPU_BUFFER_USAGE,
				indices,
				indexCount * sizeof(uint32_t),
				&indexBuffer);

			free(indices);

			if (mpgxResult != SUCCESS_MPGX_RESULT)
			{
				destroyBuffer(vertexBuffer);
				destroyImage(texture);
				free(pixels);
				return mpgxResult;
			}
		}

		if (!texture)
		{
			MpgxResult mpgxResult = setImageData(
				text->texture,
				pixels,
				vec3I(pixelLength, pixelLength, 1),
				zeroVec3I);

			free(pixels); // TODO: replace pixels with image data map

			if (mpgxResult != SUCCESS_MPGX_RESULT)
			{
				destroyBuffer(indexBuffer);
				destroyBuffer(vertexBuffer);
				destroyImage(texture);
				return mpgxResult;
			}
		}
		else
		{
#if MPGX_SUPPORT_VULKAN
			if (api == VULKAN_GRAPHICS_API)
			{
				free(text->descriptorSets);
				text->descriptorSets = descriptorSets;
			}
#endif

			destroyImage(text->texture);
			text->texture = texture;
		}

		if (!vertexBuffer)
		{
			GraphicsMesh mesh = text->mesh;
			Buffer _vertexBuffer = getGraphicsMeshVertexBuffer(mesh);

			MpgxResult mpgxResult = setBufferData(
				_vertexBuffer,
				vertices,
				vertexCount * sizeof(float),
				0);

			// TODO: if mpgxResult != SUCCESS

			setGraphicsMeshIndexCount(
				mesh,
				(vertexCount / 16) * 6);

			free(vertices);
		}
		else
		{
			GraphicsMesh mesh = text->mesh;
			Buffer _vertexBuffer = getGraphicsMeshVertexBuffer(mesh);
			Buffer _indexBuffer = getGraphicsMeshIndexBuffer(mesh);

			destroyBuffer(_vertexBuffer);
			destroyBuffer(_indexBuffer);

			setGraphicsMeshVertexBuffer(
				mesh,
				vertexBuffer);
			setGraphicsMeshIndexBuffer(
				mesh,
				UINT32_INDEX_TYPE,
				(vertexCount / 16) * 6,
				0,
				indexBuffer);
		}

		text->textSize = textSize;
	}
	else
	{
		Glyph* glyphs;
		size_t glyphCount;

		bool result = createTextGlyphs(
			data,
			dataLength,
			&glyphs,
			&glyphCount);

		if (!result)
			return UNKNOWN_ERROR_MPGX_RESULT; // TODO:

		FT_Face face = text->font->face;
		uint32_t fontSize = text->fontSize;

		FT_Error ftResult = FT_Set_Pixel_Sizes(
			face,
			0,
			(FT_UInt)fontSize);

		if (ftResult != 0)
		{
			free(glyphs);
			return UNKNOWN_ERROR_MPGX_RESULT;
		}

		float newLineAdvance =
			((float)face->size->metrics.height / 64.0f) /
			(float)fontSize;

		uint8_t* pixels;
		size_t pixelCount;
		uint32_t pixelLength;

		result = createTextPixels(
			face,
			fontSize,
			glyphs,
			glyphCount,
			0,
			&pixels,
			&pixelCount,
			&pixelLength);

		if (!result)
		{
			free(glyphs);
			return UNKNOWN_ERROR_MPGX_RESULT;
		}

		Image texture;

		MpgxResult mpgxResult = createImage(
			window,
			SAMPLED_IMAGE_TYPE,
			IMAGE_2D,
			R8_UNORM_IMAGE_FORMAT,
			(const void**)&pixels,
			vec3I(pixelLength, pixelLength, 1),
			1,
			false,
			&texture);

		free(pixels);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			free(glyphs);
			return mpgxResult;
		}

		float* vertices;
		size_t vertexCount;
		Vec2F textSize;

		result = createTextVertices(
			data,
			dataLength,
			glyphs,
			glyphCount,
			newLineAdvance,
			text->alignment,
			&vertices,
			&vertexCount,
			&textSize);

		free(glyphs);

		if (!result)
		{
			destroyImage(texture);
			return UNKNOWN_ERROR_MPGX_RESULT;
		}

		Buffer vertexBuffer;

		mpgxResult = createBuffer(
			window,
			VERTEX_BUFFER_TYPE,
			CPU_TO_GPU_BUFFER_USAGE,
			vertices,
			vertexCount * sizeof(float),
			&vertexBuffer);

		free(vertices);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			destroyImage(texture);
			return mpgxResult;
		}

		uint32_t* indices;
		size_t indexCount;

		result = createTextIndices(
			vertexCount,
			&indices,
			&indexCount);

		if (!result)
		{
			destroyBuffer(vertexBuffer);
			destroyImage(texture);
			return UNKNOWN_ERROR_MPGX_RESULT;
		}

		Buffer indexBuffer;

		mpgxResult = createBuffer(
			window,
			INDEX_BUFFER_TYPE,
			CPU_TO_GPU_BUFFER_USAGE,
			indices,
			indexCount * sizeof(uint32_t),
			&indexBuffer);

		free(indices);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			destroyBuffer(vertexBuffer);
			destroyImage(texture);
			return mpgxResult;
		}

		GraphicsMesh mesh;

		mpgxResult = createGraphicsMesh(
			window,
			UINT32_INDEX_TYPE,
			indexCount,
			0,
			vertexBuffer,
			indexBuffer,
			&mesh);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			destroyBuffer(indexBuffer);
			destroyBuffer(vertexBuffer);
			destroyImage(texture);
			return mpgxResult;
		}

#if MPGX_SUPPORT_VULKAN
		if (api == VULKAN_GRAPHICS_API)
		{
			VkWindow vkWindow = getVkWindow(window);
			VkDevice device = vkWindow->device;
			Handle handle = pipeline->vk.handle;
			uint8_t bufferCount = handle->vk.bufferCount;
			VkDescriptorPool descriptorPool = text->descriptorPool;

			VkDescriptorSet* descriptorSets;

			mpgxResult = createVkDescriptorSetArray(
				device,
				handle->vk.descriptorSetLayout,
				descriptorPool,
				bufferCount,
				handle->vk.sampler->vk.handle,
				texture->vk.imageView,
				&descriptorSets);

			if (mpgxResult != SUCCESS_MPGX_RESULT)
			{
				// TODO: destroy transform, add it to the text handle
				destroyGraphicsMesh(mesh);
				destroyImage(texture);
				return mpgxResult;
			}

			free(text->descriptorSets);

			text->descriptorPool = descriptorPool;
			text->descriptorSets = descriptorSets;
		}
#endif

		// TODO: destroy transform, add it to the text handle
		destroyGraphicsMesh(text->mesh);
		destroyImage(text->texture);

		text->texture = texture;
		text->mesh = mesh;
		text->textSize = textSize;
	}

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

	GraphicsPipeline pipeline = text->pipeline;
	Handle textPipeline = pipeline->base.handle;
	textPipeline->base.texture = text->texture;

	bool dynamicScissor = scissor.z + scissor.w != 0;

	Window window = pipeline->base.framebuffer->base.window;
	GraphicsAPI api = getGraphicsAPI();

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = getVkWindow(window);
		VkCommandBuffer commandBuffer = vkWindow->currenCommandBuffer;

		if (dynamicScissor)
		{
			VkRect2D vkScissor = {
				(int32_t)scissor.x,
				(int32_t)scissor.y,
				scissor.z,
				scissor.w,
			};
			vkCmdSetScissor(
				commandBuffer,
				0,
				1,
				&vkScissor);
		}

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline->vk.layout,
			0,
			1,
			&text->descriptorSets[vkWindow->bufferIndex],
			0,
			NULL);
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
#else
		abort();
#endif
	}
	else
	{
		abort();
	}

	return drawGraphicsMesh(
		pipeline,
		text->mesh);
}

float getTextPlatformScale(
	GraphicsPipeline textPipeline)
{
	assert(textPipeline);
	assert(strcmp(
		textPipeline->base.name,
		TEXT_PIPELINE_NAME) == 0);
	assert(textInitialized);

	Framebuffer framebuffer =
		getGraphicsPipelineFramebuffer(textPipeline);
	Vec2I framebufferSize =
		getFramebufferSize(framebuffer);
	Vec2I windowSize = getWindowSize(
		getFramebufferWindow(framebuffer));

	return max(
		(float)framebufferSize.x / (float)windowSize.x,
		(float)framebufferSize.y / (float)windowSize.y);
}

MpgxResult createTextSampler(
	Window window,
	Sampler* textSampler)
{
	assert(window);
	assert(textSampler);

	return createSampler(
		window,
		LINEAR_IMAGE_FILTER,
		LINEAR_IMAGE_FILTER,
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
		sizeof(Vec2F) * 2,
		VK_VERTEX_INPUT_RATE_VERTEX,
	},
};
static const VkVertexInputAttributeDescription vertexInputAttributeDescriptions[2] = {
	{
		0,
		0,
		VK_FORMAT_R32G32_SFLOAT,
		0,
	},
	{
		1,
		0,
		VK_FORMAT_R32G32_SFLOAT,
		sizeof(Vec2F),
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

static void onVkUniformsSet(GraphicsPipeline graphicsPipeline)
{
	assert(graphicsPipeline);

	Handle handle = graphicsPipeline->vk.handle;
	VkWindow vkWindow = getVkWindow(handle->vk.window);
	VkCommandBuffer commandBuffer = vkWindow->currenCommandBuffer;
	VkPipelineLayout layout = graphicsPipeline->vk.layout;

	vkCmdPushConstants(
		commandBuffer,
		layout,
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		sizeof(VertexPushConstants),
		&handle->vk.vpc);
	vkCmdPushConstants(
		commandBuffer,
		layout,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		sizeof(VertexPushConstants),
		sizeof(FragmentPushConstants),
		&handle->vk.fpc);
}
static MpgxResult onVkResize(
	GraphicsPipeline graphicsPipeline,
	Vec2I newSize,
	void* createData)
{
	assert(graphicsPipeline);
	assert(newSize.x > 0);
	assert(newSize.y > 0);
	assert(createData);

	Handle handle = graphicsPipeline->vk.handle;
	Window window = handle->vk.window;
	VkWindow vkWindow = getVkWindow(window);
	uint32_t bufferCount = vkWindow->swapchain->bufferCount;

	if (bufferCount != handle->vk.bufferCount)
	{
		Text* texts = handle->vk.texts;
		size_t textCount = handle->vk.textCount;
		VkDevice device = vkWindow->device;

		for (size_t i = 0; i < textCount; i++)
		{
			Text text = texts[i];

			VkDescriptorPool descriptorPool;

			MpgxResult mpgxResult = createVkDescriptorPoolInstance(
				device,
				bufferCount,
				&descriptorPool);

			if (mpgxResult != SUCCESS_MPGX_RESULT)
				return mpgxResult;

			VkDescriptorSet* descriptorSets;

			mpgxResult = createVkDescriptorSetArray(
				device,
				handle->vk.descriptorSetLayout,
				descriptorPool,
				bufferCount,
				handle->vk.sampler->vk.handle,
				text->texture->vk.imageView,
				&descriptorSets);

			if (mpgxResult != SUCCESS_MPGX_RESULT)
			{
				vkDestroyDescriptorPool(
					device,
					descriptorPool,
					NULL);
				return mpgxResult;
			}

			free(text->descriptorSets);

			vkDestroyDescriptorPool(
				device,
				text->descriptorPool,
				NULL);

			text->descriptorPool = descriptorPool;
			text->descriptorSets = descriptorSets;

		}

		handle->vk.bufferCount = bufferCount;
	}

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
		2,
		vertexInputAttributeDescriptions,
		1,
		&handle->vk.descriptorSetLayout,
		2,
		pushConstantRanges,
	};

	*(VkGraphicsPipelineCreateData*)createData = _createData;
	return SUCCESS_MPGX_RESULT;
}
static void onVkDestroy(void* _handle)
{
	Handle handle = _handle;

	if (!handle)
		return;

	assert(handle->vk.textCount == 0);

	VkWindow vkWindow = getVkWindow(handle->vk.window);
	VkDevice device = vkWindow->device;

	vkDestroyDescriptorSetLayout(
		device,
		handle->vk.descriptorSetLayout,
		NULL);
	free(handle->vk.texts);
	free(handle);
}
inline static MpgxResult createVkPipeline(
	Framebuffer framebuffer,
	const GraphicsPipelineState* state,
	Handle handle,
	Shader* shaders,
	uint8_t shaderCount,
	GraphicsPipeline* graphicsPipeline)
{
	assert(framebuffer);
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
		onVkDestroy(handle);
		return vkToMpgxResult(vkResult);
	}

	VkGraphicsPipelineCreateData createData = {
		1,
		vertexInputBindingDescriptions,
		2,
		vertexInputAttributeDescriptions,
		1,
		&descriptorSetLayout,
		2,
		pushConstantRanges,
	};

	handle->vk.descriptorSetLayout = descriptorSetLayout;
	handle->vk.bufferCount = vkWindow->swapchain->bufferCount;

	return createGraphicsPipeline(
		framebuffer,
		TEXT_PIPELINE_NAME,
		state,
		NULL,
		onVkUniformsSet,
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
		(const float*)&handle->gl.fpc.color);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(
		0,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec2F) * 2,
		0);
	glVertexAttribPointer(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec2F) * 2,
		(const void*)sizeof(Vec2F));

	glUniform1i(
		handle->gl.textureLocation,
		0);

	glActiveTexture(GL_TEXTURE0);

	glBindTexture(
		GL_TEXTURE_2D,
		handle->gl.texture->gl.handle);
	glBindSampler(
		0,
		handle->gl.sampler->gl.handle);

	assertOpenGL();
}
static MpgxResult onGlResize(
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
	return SUCCESS_MPGX_RESULT;
}
static void onGlDestroy(void* _handle)
{
	Handle handle = _handle;

	if (!handle)
		return;

	assert(handle->gl.textCount == 0);

	free(handle->gl.texts);
	free(handle);
}
inline static MpgxResult createGlPipeline(
	Framebuffer framebuffer,
	const GraphicsPipelineState* state,
	Handle handle,
	Shader* shaders,
	uint8_t shaderCount,
	GraphicsPipeline* graphicsPipeline)
{
	assert(framebuffer);
	assert(state);
	assert(handle);
	assert(shaders);
	assert(shaderCount > 0);
	assert(graphicsPipeline);

	GraphicsPipeline graphicsPipelineInstance;

	MpgxResult mpgxResult = createGraphicsPipeline(
		framebuffer,
		TEXT_PIPELINE_NAME,
		state,
		NULL,
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
		onGlDestroy(handle);
		return mpgxResult;
	}

	GLuint glHandle = graphicsPipelineInstance->gl.glHandle;

	GLint mvpLocation, colorLocation, textureLocation;

	bool result = getGlUniformLocation(
		glHandle,
		"u_MVP",
		&mvpLocation);
	result &= getGlUniformLocation(
		glHandle,
		"u_Color",
		&colorLocation);
	result &= getGlUniformLocation(
		glHandle,
		"u_Texture",
		&textureLocation);

	if (!result)
	{
		destroyGraphicsPipeline(graphicsPipelineInstance);
		return BAD_SHADER_CODE_MPGX_RESULT;
	}

	assertOpenGL();

	handle->gl.mvpLocation = mvpLocation;
	handle->gl.colorLocation = colorLocation;
	handle->gl.textureLocation = textureLocation;

	*graphicsPipeline = graphicsPipelineInstance;
	return SUCCESS_MPGX_RESULT;
}
#endif

MpgxResult createTextPipelineExt(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Sampler sampler,
	const GraphicsPipelineState* state,
	size_t textCapacity,
	GraphicsPipeline* textPipeline)
{
	assert(framebuffer);
	assert(vertexShader);
	assert(fragmentShader);
	assert(sampler);
	assert(state);
	assert(textCapacity > 0);
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
		textCapacity * sizeof(Text));

	if (!texts)
	{
		free(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	Window window = framebuffer->base.window;
	handle->base.window = window;
	handle->base.texture = NULL;
	handle->base.sampler = sampler;
	handle->base.vpc.mvp = identMat4F;
	handle->base.fpc.color = whiteLinearColor;
	handle->base.texts = texts;
	handle->base.textCapacity = textCapacity;
	handle->base.textCount = 0;

	Shader shaders[2] = {
		vertexShader,
		fragmentShader,
	};

	GraphicsAPI api = getGraphicsAPI();

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		return createVkPipeline(
			framebuffer,
			state,
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
			state,
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
MpgxResult createTextPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Sampler sampler,
	size_t textCapacity,
	bool useScissor,
	GraphicsPipeline* textPipeline)
{
	assert(framebuffer);
	assert(vertexShader);
	assert(fragmentShader);
	assert(sampler);
	assert(textCapacity > 0);
	assert(textPipeline);

	Vec2I framebufferSize =
		framebuffer->base.size;
	Vec4I size = vec4I(0, 0,
		framebufferSize.x,
		framebufferSize.y);

	GraphicsPipelineState state = {
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
		true,
		false,
		false,
		true,
		false,
		false,
		DEFAULT_LINE_WIDTH,
		size,
		useScissor ? zeroVec4I : size,
		defaultDepthRange,
		defaultDepthBias,
		defaultBlendColor,
	};

	return createTextPipelineExt(
		framebuffer,
		vertexShader,
		fragmentShader,
		sampler,
		&state,
		textCapacity,
		textPipeline);
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
