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
#include "cmmt/color.h"
#include "cmmt/bounding.h"

#include <stdbool.h>

#define TEXT_PIPELINE_NAME "Text"

/*
 * Font structure.
 */
typedef struct Font_T Font_T;
/*
 * Font instance.
 */
typedef Font_T* Font;

/*
 * Font atlas structure.
 */
typedef struct FontAtlas_T FontAtlas_T;
/*
 * Font atlas instance.
 */
typedef FontAtlas_T* FontAtlas;

/*
 * Text structure.
 */
typedef union Text_T Text_T;
/*
 * Text instance.
 */
typedef Text_T* Text;

/*
 * Alignment types.
 */
typedef enum AlignmentType_T
{
	CENTER_ALIGNMENT_TYPE = 0,
	LEFT_ALIGNMENT_TYPE = 1,
	RIGHT_ALIGNMENT_TYPE = 2,
	BOTTOM_ALIGNMENT_TYPE = 3,
	TOP_ALIGNMENT_TYPE = 4,
	LEFT_BOTTOM_ALIGNMENT_TYPE = 5,
	LEFT_TOP_ALIGNMENT_TYPE = 6,
	RIGHT_BOTTOM_ALIGNMENT_TYPE = 7,
	RIGHT_TOP_ALIGNMENT_TYPE = 8,
	ALIGNMENT_TYPE_COUNT = 9,
} AlignmentType_T;
/*
 * Alignment type.
 */
typedef uint8_t AlignmentType;

/*
 * Text pipeline enumeration function.
 */
typedef void(*OnPipelineText)(
	Text text, void* handle);

/*
 * String containing all printable ASCII UTF-8 characters.
 */
#define printableAscii " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_abcdefghijklmnopqrstuvwxyz{|}~\0"

/*
 * String containing all printable ASCII UTF-32 characters.
 */
static const uint32_t printableAscii32[] = {
	' ', '!', '\"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?', '@',
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_', '`',
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
	'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', '\0',
};

/*
 * Initialize text subsystem.
 * Returns true on success.
 *
 * logger - logger instance or NULL.
 */
bool initializeText(Logger logger);
/*
 * Terminates text subsystem.
 * logger - logger instance or NULL.
 */
void terminateText(Logger logger);
/*
 * Returns true if text subsystems are initialized.
 */
bool isTextInitialized();

/*
 * Convert UTF-8 string to UTF-32.
 * Returns string length on success, otherwise 0.
 *
 * source - source UTF-8 string.
 * destination - destination UTF-32 string.
 * length - source string length.
 */
size_t stringUTF8toUTF32(
	const char* source,
	size_t sourceLength,
	uint32_t* destination);

/*
 * Allocate a new UTF-8 string from the UTF-32 string.
 * Returns operation MPGX result.
 *
 * source - source UTF-32 string.
 * sourceLength - source string length.
 * destination - pointer to the destination UTF-8 string.
 * destinationLength - pointer to the destination string length.
 */
MpgxResult allocateStringUTF8(
	const uint32_t* source,
	size_t sourceLength,
	char** destination,
	size_t* destinationLength);
/*
 * Returns true if UTF-8 string is valid.
 *
 * string - UTF-8 string.
 * stringLength - string length.
 */
bool validateStringUTF8(
	const char* string,
	size_t stringLength);

/*
 * Allocate a new UTF-32 string from the UTF-8 string.
 * Returns operation MPGX result.
 *
 * source - source UTF-8 string.
 * stringLength - source string length.
 * destination - pointer to the destination UTF-32 string.
 * destinationLength - pointer to the destination string length.
 */
MpgxResult allocateStringUTF32(
	const char* source,
	size_t sourceLength,
	uint32_t** destination,
	size_t* destinationLength);
/*
 * Returns true if UTF-32 string is valid.
 *
 * string - UTF-32 string.
 * stringLength - string length.
 */
bool validateStringUTF32(
	const uint32_t* string,
	size_t stringLength);

/*
 * Create a new font instance.
 * Returns font instance on success, otherwise NULL.
 *
 * data - font data array.
 * size - data array size.
 * index - font face index.
 * logger - logger instance or NULL.
 */
Font createFont(
	const void* data,
	size_t size,
	size_t index,
	Logger logger);
/*
 * Create a new font instance from the file.
 * Returns font instance on success, otherwise NULL.
 *
 * path - file path string.
 * index - font face index.
 * logger - logger instance or NULL.
 */
Font createFontFromFile(
	const char* path,
	size_t index,
	Logger logger);
/*
 * Create a new font instance from the pack data.
 * Returns font instance on success, otherwise NULL.
 *
 * path - font data item path string.
 * index - font face index.
 * packReader - pack reader instance.
 * logger - logger instance or NULL.
 */
Font createFontFromPack(
	const char* path,
	size_t index,
	PackReader packReader,
	Logger logger);
/*
 * Destroys font instance.
 * font - font instance or NULL.
 */
void destroyFont(Font font);

/*
 * Create a new UTF-32 font atlas instance.
 * Returns operation MPGX result.
 *
 * textPipeline - text pipeline instance.
 * regularFonts - regular font array.
 * boldFonts - bold font array.
 * italicFonts - italic font array.
 * boldItalicFonts - bold italic font array.
 * fontCount - font array size.
 * fontSize - font pixel size.
 * chars - atlas char array.
 * charCount - char array size.
 * logger - logger instance or NULL.
 * fontAtlas - pointer to the font atlas instance.
 */
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
	FontAtlas* fontAtlas);
/*
 * Create a new UTF-8 font atlas instance.
 * Returns operation MPGX result.
 *
 * textPipeline - text pipeline instance.
 * regularFonts - regular font array.
 * boldFonts - bold font array.
 * italicFonts - italic font array.
 * boldItalicFonts - bold italic font array.
 * fontCount - font array size.
 * fontSize - font pixel size.
 * chars - atlas char array.
 * charCount - char array size.
 * logger - logger instance or NULL.
 * fontAtlas - pointer to the font atlas instance.
 */
MpgxResult createFontAtlas8(
	GraphicsPipeline textPipeline,
	Font* regularFonts,
	Font* boldFonts,
	Font* italicFonts,
	Font* boldItalicFonts,
	size_t fontCount,
	uint32_t fontSize,
	const char* chars,
	size_t charCount,
	Logger logger,
	FontAtlas* fontAtlas);
/*
 * Create a new ASCII font atlas instance.
 * Returns operation MPGX result.
 *
 * textPipeline - text pipeline instance.
 * regularFonts - regular font array.
 * boldFonts - bold font array.
 * italicFonts - italic font array.
 * boldItalicFonts - bold italic font array.
 * fontCount - font array size.
 * fontSize - font pixel size.
 * logger - logger instance or NULL.
 * fontAtlas - pointer to the font atlas instance.
 */
MpgxResult createAsciiFontAtlas(
	GraphicsPipeline textPipeline,
	Font* regularFonts,
	Font* boldFonts,
	Font* italicFonts,
	Font* boldItalicFonts,
	size_t fontCount,
	uint32_t fontSize,
	Logger logger,
	FontAtlas* fontAtlas);
/*
 * Destroys font atlas instance.
 * fontAtlas - font atlas instance or NULL.
 */
void destroyFontAtlas(FontAtlas fontAtlas);

/*
 * Returns font atlas text graphics pipeline.
 * fontAtlas - font atlas instance.
 */
GraphicsPipeline getFontAtlasPipeline(FontAtlas fontAtlas);
/*
 * Returns font atlas regular font array.
 * fontAtlas - font atlas instance.
 */
Font* getFontAtlasRegularFonts(FontAtlas fontAtlas);
/*
 * Returns font atlas bold font array.
 * fontAtlas - font atlas instance.
 */
Font* getFontAtlasBoldFonts(FontAtlas fontAtlas);
/*
 * Returns font atlas italic font array.
 * fontAtlas - font atlas instance.
 */
Font* getFontAtlasItalicFonts(FontAtlas fontAtlas);
/*
 * Returns font atlas bold italic font array.
 * fontAtlas - font atlas instance.
 */
Font* getFontAtlasBoldItalicFonts(FontAtlas fontAtlas);
/*
 * Returns font atlas font array size.
 * fontAtlas - font atlas instance.
 */
size_t getFontAtlasFontCount(FontAtlas fontAtlas);
/*
 * Returns font atlas font pixel size.
 * fontAtlas - font atlas instance.
 */
uint32_t getFontAtlasFontSize(FontAtlas fontAtlas);
/*
 * Returns font atlas logger.
 * fontAtlas - font atlas instance.
 */
Logger getFontAtlasLogger(FontAtlas fontAtlas);
/*
 * Returns true if font atlas is auto generated.
 * fontAtlas - font atlas instance.
 */
bool isFontAtlasGenerated(FontAtlas fontAtlas);

// TODO: shrinkAtlasIndexBuffer

/*
 * Create a new UTF-32 atlas text instance.
 * Returns operation MPGX result.
 *
 * fontAtlas - font atlas instance.
 * string - text string or NULL.
 * length - string length or 0.
 * alignment - text alignment.
 * color - initial text color.
 * isBold - is text bold initially.
 * isItalic - is text italic initially.
 * useTags - use HTML tags.
 * isConstant - is text constant.
 * text - pointer to the text instance.
 */
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
	Text* text);
/*
 * Create a new UTF-8 atlas text instance.
 * Returns operation MPGX result.
 *
 * fontAtlas - font atlas instance.
 * string - text string or NULL.
 * length - string length or 0.
 * alignment - text alignment.
 * color - initial text color.
 * isBold - is text bold initially.
 * isItalic - is text italic initially.
 * useTags - use HTML tags.
 * isConstant - is text constant.
 * text - pointer to the text instance.
 */
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
	Text* text);

/*
 * Create a new UTF-32 font text instance.
 * Returns operation MPGX result.
 *
 * textPipeline - text pipeline instance.
 * regularFonts - regular font array.
 * boldFonts - bold font array.
 * italicFonts - italic font array.
 * boldItalicFonts - bold italic font array.
 * fontCount - font array size.
 * fontSize - font pixel size.
 * string - text string or NULL.
 * length - string length or 0.
 * alignment - text alignment.
 * color - initial text color.
 * isBold - is text bold initially.
 * isItalic - is text italic initially.
 * useTags - use HTML tags.
 * isConstant - is text constant.
 * logger - logger instance or NULL.
 * text - pointer to the text instance.
 */
MpgxResult createFontText(
	GraphicsPipeline textPipeline,
	Font* regularFonts,
	Font* boldFonts,
	Font* italicFonts,
	Font* boldItalicFonts,
	size_t fontCount,
	uint32_t fontSize,
	const uint32_t* string,
	size_t length,
	AlignmentType alignment,
	SrgbColor color,
	bool isBold,
	bool isItalic,
	bool useTags,
	bool isConstant,
	Logger logger,
	Text* text);
/*
 * Create a new UTF-8 font text instance.
 * Returns operation MPGX result.
 *
 * textPipeline - text pipeline instance.
 * regularFonts - regular font array.
 * boldFonts - bold font array.
 * italicFonts - italic font array.
 * boldItalicFonts - bold italic font array.
 * fontCount - font array size.
 * fontSize - font pixel size.
 * string - text string or NULL.
 * length - string length or 0.
 * alignment - text alignment.
 * color - initial text color.
 * isBold - is text bold initially.
 * isItalic - is text italic initially.
 * useTags - use HTML tags.
 * isConstant - is text constant.
 * logger - logger instance or NULL.
 * text - pointer to the text instance.
 */
MpgxResult createFontText8(
	GraphicsPipeline textPipeline,
	Font* regularFonts,
	Font* boldFonts,
	Font* italicFonts,
	Font* boldItalicFonts,
	size_t fontCount,
	uint32_t fontSize,
	const char* string,
	size_t length,
	AlignmentType alignment,
	SrgbColor color,
	bool isBold,
	bool isItalic,
	bool useTags,
	bool isConstant,
	Logger logger,
	Text* text);

/*
 * Destroys text instance.
 * text - text instance or NULL.
 */
void destroyText(Text text);

/*
 * Returns text font atlas.
 * text - text instance.
 */
FontAtlas getTextFontAtlas(Text text);
/*
 * Returns text mesh size.
 * text - text instance.
 */
Vec2F getTextSize(Text text);
/*
 * Returns true if text is constant.
 * text - text instance.
 */
bool isTextConstant(Text text);

/*
 * Returns text string.
 * text - text instance.
 */
const uint32_t* getTextString(Text text);
/*
 * Returns text string length.
 * text - text instance.
 */
size_t getTextLength(Text text);

/*
 * Set text UTF-32 string.
 * Returns true on success.
 *
 * text - text instance.
 * string - text string or NULL.
 * length - string length or 0.
 */
bool setTextString(
	Text text,
	const uint32_t* string,
	size_t length);
/*
 * Set text UTF-8 string.
 * Returns true on success.
 *
 * text - text instance.
 * string - text string or NULL.
 * length - string length or 0.
 */
bool setTextString8(
	Text text,
	const char* string,
	size_t length);

/*
 * Addend text UTF-32 string.
 * Returns true on success.
 *
 * text - text instance.
 * string - text string.
 * length - string length.
 * index - insert index.
 */
bool appendTextString32(
	Text text,
	const uint32_t* string,
	size_t length,
	size_t index);
/*
 * Remove text UTF-32 char at index.
 * Return true on success.
 *
 * text - text instance.
 * index - char index.
 */
void removeTextChar(
	Text text,
	size_t index);

/*
 * Returns text alignment type.
 * text - text instance.
 */
AlignmentType getTextAlignment(Text text);
/*
 * Sets text alignment type.
 *
 * text - text instance.
 * alignment - alignment type.
 */
void setTextAlignment(
	Text text,
	AlignmentType alignment);

/*
 * Returns text color value.
 * text - text instance.
 */
SrgbColor getTextColor(Text text);
/*
 * Sets text color value.
 *
 * text - text instance.
 * color - color value.
 */
void setTextColor(
	Text text,
	SrgbColor color);

/*
 * Returns true if text is bold initially.
 * text - text instance.
 */
bool isTextBold(Text text);
/*
 * Sets text bold initially.
 *
 * text - text instance.
 * isBold - is text bold.
 */
void setTextBold(
	Text text,
	bool isBold);

/*
 * Returns true if text is italic initially.
 * text - text instance.
 */
bool isTextItalic(Text text);
/*
 * Sets text italic initially.
 *
 * text - text instance.
 * isItalic - is text italic.
 */
void setTextItalic(
	Text text,
	bool isItalic);

/*
 * Returns true if text uses HTML tags.
 * text - text instance.
 */
bool isTextUseTags(Text text);
/*
 * Sets text use HTML tags.
 *
 * text - text instance.
 * useTags - use HTML tags.
 */
void setTextUseTags(
	Text text,
	bool useTags);

/*
 * Returns text atlas font size.
 * text - text instance.
 */
uint32_t getTextFontSize(Text text);
/*
 * Sets text atlas font size.
 *
 * text - text instance.
 * fontSize - font size value.
 */
void setTextFontSize(
	Text text,
	uint32_t fontSize);

/*
 * Get text cursor advance.
 * Returns true on success.
 *
 * text - text instance.
 * index - cursor index.
 * advance - pointer to the advance.
 */
bool getTextCursorAdvance(
	Text text,
	size_t index,
	Vec2F* advance);
/*
 * Get text cursor index.
 * Returns true on success.
 *
 * text - text instance.
 * advance - cursor advance.
 * index - pointer to the index.
 */
bool getTextCursorIndex(
	Text text,
	Vec2F advance,
	size_t* index);

/*
 * Recreate text mesh data.
 * Returns operation MPGX result.
 */
MpgxResult bakeText(Text text);
/*
 * Draw text mesh. (rendering command)
 * Returns drawn index count.
 *
 * text - text instance.
 */
size_t drawText(Text text);

// TODO: shrink text buffers

/*
 * Create a new text image sampler.
 * Returns operation MPGX result.
 *
 * window - window instance.
 * textSampler - pointer to the text sampler instance.
 */
MpgxResult createTextSampler(
	Window window,
	Sampler* textSampler);

/*
 * Create a new text pipeline instance.
 * Returns operation MPGX result.
 *
 * framebuffer - framebuffer instance.
 * vertexShader - text vertex shader instance.
 * fragmentShader - text fragment shader instance.
 * sampler - image sampler instance.
 * state - sprite pipeline state or NULL.
 * useScissors - use scissors for text rendering.
 * capacity - initial text array capacity.
 * textPipeline - pointer to the text pipeline.
 */
MpgxResult createTextPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Sampler sampler,
	const GraphicsPipelineState* state,
	bool useScissors,
	size_t capacity,
	GraphicsPipeline* textPipeline);

/*
 * Returns text pipeline image sampler.
 * textPipeline - text pipeline instance.
 */
Sampler getTextPipelineSampler(GraphicsPipeline textPipeline);
/*
 * Returns text pipeline text count.
 * textPipeline - text pipeline instance.
 */
size_t getTextPipelineCount(GraphicsPipeline textPipeline);

/*
 * Returns text pipeline MVP matrix.
 * textPipeline - text pipeline instance.
 */
const mat4* getTextPipelineMVP(
	GraphicsPipeline textPipeline);
/*
 * Sets text pipeline image sampler.
 *
 * textPipeline - text pipeline instance.
 * mvp - model view projection matrix value.
 */
void setTextPipelineMVP(
	GraphicsPipeline textPipeline,
	const Mat4F* mvp);

/*
 * Returns text pipeline MVP matrix.
 * textPipeline - text pipeline instance.
 */
vec4 getTextPipelineColor(
	GraphicsPipeline textPipeline);
/*
 * Sets text pipeline image sampler.
 *
 * textPipeline - text pipeline instance.
 * color - color value.
 */
void setTextPipelineColor(
	GraphicsPipeline textPipeline,
	LinearColor color);

/*
 * Enumerates pipeline texts.
 *
 * textPipeline - text pipeline instance.
 * onText - on pipeline text function.
 * handle - function argument or NULL.
 */
void enumeratePipelineTexts(
	GraphicsPipeline textPipeline,
	OnPipelineText onText,
	void* handle);

/*
 * Returns running platform scale.
 * framebuffer - framebuffer instance.
 */
inline static cmmt_float_t getPlatformScale(Framebuffer framebuffer)
{
	assert(framebuffer);

	Vec2I framebufferSize = getFramebufferSize(framebuffer);
	Vec2I windowSize = getWindowSize(getFramebufferWindow(framebuffer));

	return max(
		(cmmt_float_t)framebufferSize.x / (cmmt_float_t)windowSize.x,
		(cmmt_float_t)framebufferSize.y / (cmmt_float_t)windowSize.y);
}
/*
 * Returns running platform font size.
 *
 * scale - platform scale.
 * fontSize - font size in pixels.
 */
inline static uint32_t getPlatformFontSize(
	cmmt_float_t platformScale,
	uint32_t fontSize)
{
	fontSize = (uint32_t)((cmmt_float_t)fontSize * platformScale);
	if (fontSize % 2 != 0) fontSize += 1;
	return fontSize;
}

/*
 * Creates 2D text bounding box.
 *
 * alignment - alignment type.
 * textSize - text size value.
 */
inline static Box2F createTextBox2F(
	AlignmentType alignment,
	Vec2F textSize)
{
	assert(alignment < ALIGNMENT_TYPE_COUNT);

	Vec2F position;

	switch (alignment)
	{
	default:
		abort();
	case CENTER_ALIGNMENT_TYPE:
		position = zeroVec2F;
		break;
	case LEFT_ALIGNMENT_TYPE:
		position = vec2F(
			(cmmt_float_t)textSize.x * (cmmt_float_t)0.5,
			(cmmt_float_t)0.0);
		break;
	case RIGHT_ALIGNMENT_TYPE:
		position = vec2F(
			(cmmt_float_t)textSize.x * (cmmt_float_t)-0.5,
			(cmmt_float_t)0.0);
		break;
	case BOTTOM_ALIGNMENT_TYPE:
		position = vec2F(
			(cmmt_float_t)0.0,
			(cmmt_float_t)textSize.y * (cmmt_float_t)0.5);
		break;
	case TOP_ALIGNMENT_TYPE:
		position = vec2F(
			(cmmt_float_t)0.0,
			(cmmt_float_t)textSize.y * (cmmt_float_t)-0.5);
		break;
	case LEFT_BOTTOM_ALIGNMENT_TYPE:
		position = vec2F(
			(cmmt_float_t)textSize.x * (cmmt_float_t)0.5,
			(cmmt_float_t)textSize.y * (cmmt_float_t)0.5);
		break;
	case LEFT_TOP_ALIGNMENT_TYPE:
		position = vec2F(
			(cmmt_float_t)textSize.x * (cmmt_float_t)0.5,
			(cmmt_float_t)textSize.y * (cmmt_float_t)-0.5);
		break;
	case RIGHT_BOTTOM_ALIGNMENT_TYPE:
		position = vec2F(
			(cmmt_float_t)textSize.x * (cmmt_float_t)-0.5,
			(cmmt_float_t)textSize.y * (cmmt_float_t)0.5);
		break;
	case RIGHT_TOP_ALIGNMENT_TYPE:
		position = vec2F(
			(cmmt_float_t)textSize.x * (cmmt_float_t)-0.5,
			(cmmt_float_t)textSize.y * (cmmt_float_t)-0.5);
		break;
	}

	return posSizeBox2F(position, textSize);
}
/*
 * Creates 3D text bounding box.
 *
 * alignment - alignment type.
 * textSize - text size value.
 */
inline static Box3F createTextBox3F(
	AlignmentType alignment,
	Vec2F textSize)
{
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	Box2F box = createTextBox2F(alignment, textSize);

	return box3F(
		vec3F(box.minimum.x, box.minimum.y, (cmmt_float_t)-0.5),
		vec3F(box.maximum.x, box.maximum.y, (cmmt_float_t)0.5));
}

// TODO: create inside texture square for the underline sampling first full symbol.
