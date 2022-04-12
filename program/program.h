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

// =================================================================
// Modify this file to create your custom game or application logic.
// =================================================================

#pragma once
#include "uran/defines.h"
#include "uran/editor.h"
#include "uran/shader_data.h"
#include "uran/primitives/square_primitive.h"

#include "conf/reader.h"
#include "conf/writer.h"

#include <string.h>

#define SETTINGS_FILE_PATH "editor-settings.txt"
#define RESOURCES_FILE_PATH "editor-resources.pack"
#define ENGINE_NAME "Uran"
#define APPLICATION_NAME "Editor"
#define WINDOW_TITLE "Uran Editor"

typedef struct Settings
{
	GraphicsAPI graphicsAPI;
} Settings;
typedef struct Program_T
{
	Logger logger;
	ThreadPool threadPool;
	Transformer transformer;
	Window window;
	GraphicsPipeline panelPipeline;
	GraphicsPipeline textPipeline;
	FontAtlas fontAtlas;
	UserInterface ui;
	Editor editor;
	Settings settings;
} Program_T;

typedef Program_T* Program;

inline static void loadSettings(
	Logger logger,
	Settings* _settings)
{
	assert(logger);
	assert(_settings);

	Settings settings;

	ConfReader confReader;
	size_t errorLine;

	ConfResult confResult = createConfFileReader(
		SETTINGS_FILE_PATH,
		&confReader,
		&errorLine);

	if (confResult != SUCCESS_CONF_RESULT)
	{
		logMessage(logger, WARN_LOG_LEVEL,
			"Failed to read settings. (error: %s at line %zu)",
			confResultToString(confResult), errorLine);

		settings.graphicsAPI = VULKAN_GRAPHICS_API;
		*_settings = settings;
		return;
	}

	const char* stringValue;

	bool result = getConfReaderString(confReader,
		"graphicsAPI", &stringValue, NULL);

	if (!result)
	{
		logMessage(logger, WARN_LOG_LEVEL,
			"Failed to read \"graphicsAPI\" settings value.");
		settings.graphicsAPI = VULKAN_GRAPHICS_API;
	}
	else
	{
		if (strcmp(stringValue, "vulkan") == 0)
			settings.graphicsAPI = VULKAN_GRAPHICS_API;
		else if (strcmp(stringValue, "opengl") == 0)
			settings.graphicsAPI = OPENGL_GRAPHICS_API;
		else if (strcmp(stringValue, "opengles") == 0)
			settings.graphicsAPI = OPENGL_ES_GRAPHICS_API;
		else
		{
			logMessage(logger, WARN_LOG_LEVEL,
				"Unknown \"graphicsAPI\" settings value.");
			settings.graphicsAPI = VULKAN_GRAPHICS_API;
		}
	}

	destroyConfReader(confReader);
	*_settings = settings;

	logMessage(logger, INFO_LOG_LEVEL,
		"Loaded settings.");
}
inline static void storeSettings(
	Logger logger,
	Settings settings)
{
	assert(logger);

	ConfWriter confWriter;

	ConfResult confResult = createConfFileWriter(
		SETTINGS_FILE_PATH,
		&confWriter);

	if (confResult != SUCCESS_CONF_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create settings writer. (error: %s)",
			confResultToString(confResult));
		return;
	}

	bool result = writeConfComment(confWriter,
		"Uran Editor - Settings (v" URAN_VERSION_STRING "). Apache-2.0 License.");
	result &= writeConfComment(confWriter,
		"Copyright 2022 Nikita Fediuchin. All rights reserved.");
	result &= writeConfNewLine(confWriter);

	const char* stringValue;

	if (settings.graphicsAPI == VULKAN_GRAPHICS_API)
		stringValue = "vulkan";
	else if (settings.graphicsAPI == OPENGL_GRAPHICS_API)
		stringValue = "opengl";
	else if (settings.graphicsAPI == OPENGL_ES_GRAPHICS_API)
		stringValue = "opengles";
	else
		abort();

	result &= writeConfString(confWriter,
		"graphicsAPI", stringValue);
	//result &= writeConfNewLine(confWriter);

	destroyConfWriter(confWriter);

	if (!result)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to write settings values.");
		return;
	}

	logMessage(logger, INFO_LOG_LEVEL,
		"Stored settings.");
}

inline static GraphicsPipeline createPanelPipelineInstance(
	Logger logger,
	PackReader packReader,
	Framebuffer framebuffer)
{
	assert(logger);
	assert(packReader);
	assert(framebuffer);

	GraphicsAPI api = getGraphicsAPI();

	const void* vertexPath;
	const void* fragmentPath;

	if (api == VULKAN_GRAPHICS_API)
	{
		vertexPath = "shaders/vulkan/panel.vert.spv";
		fragmentPath = "shaders/vulkan/panel.frag.spv";
	}
	else if (api == OPENGL_GRAPHICS_API ||
			 api == OPENGL_ES_GRAPHICS_API)
	{
		vertexPath = "shaders/opengl/panel.vert";
		fragmentPath = "shaders/opengl/panel.frag";
	}
	else
	{
		abort();
	}

	Window window = getFramebufferWindow(framebuffer);

	Shader vertexShader = createShaderFromPack(
		packReader,
		vertexPath,
		window,
		VERTEX_SHADER_TYPE,
		logger);

	if (!vertexShader)
		return NULL;

	Shader fragmentShader = createShaderFromPack(
		packReader,
		fragmentPath,
		window,
		FRAGMENT_SHADER_TYPE,
		logger);

	if (!fragmentShader)
	{
		destroyShader(vertexShader);
		return NULL;
	}

	Buffer vertexBuffer;

	MpgxResult mpgxResult = createBuffer(
		window,
		VERTEX_BUFFER_TYPE,
		GPU_ONLY_BUFFER_USAGE,
		oneSquareVertices2D,
		sizeof(oneSquareVertices2D),
		&vertexBuffer);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create panel vertex buffer. "
			"(error: %s)", mpgxResultToString(mpgxResult));
		destroyShader(fragmentShader);
		destroyShader(vertexShader);
		return NULL;
	}

	Buffer indexBuffer;

	mpgxResult = createBuffer(
		window,
		INDEX_BUFFER_TYPE,
		GPU_ONLY_BUFFER_USAGE,
		triangleSquareIndices,
		sizeof(triangleSquareIndices),
		&indexBuffer);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create panel index buffer. "
			"(error: %s)", mpgxResultToString(mpgxResult));
		destroyBuffer(vertexBuffer);
		destroyShader(fragmentShader);
		destroyShader(vertexShader);
		return NULL;
	}

	GraphicsMesh mesh;

	mpgxResult = createGraphicsMesh(
		window,
		UINT16_INDEX_TYPE,
		sizeof(triangleSquareIndices) / sizeof(uint16_t),
		0,
		vertexBuffer,
		indexBuffer,
		&mesh);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create panel mesh. "
			"(error: %s)", mpgxResultToString(mpgxResult));
		destroyBuffer(indexBuffer);
		destroyBuffer(vertexBuffer);
		destroyShader(fragmentShader);
		destroyShader(vertexShader);
		return NULL;
	}

	GraphicsPipeline pipeline;

	mpgxResult = createPanelPipeline(
		framebuffer,
		vertexShader,
		fragmentShader,
		mesh,
		NULL,
		&pipeline);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create panel pipeline. "
			"(error: %s)", mpgxResultToString(mpgxResult));
		destroyGraphicsMesh(mesh);
		destroyBuffer(indexBuffer);
		destroyBuffer(vertexBuffer);
		destroyShader(fragmentShader);
		destroyShader(vertexShader);
		return NULL;
	}

	return pipeline;
}
inline static void destroyPanelPipelineInstance(
	GraphicsPipeline panelPipeline)
{
	if (!panelPipeline)
		return;

	GraphicsMesh mesh = getPanelPipelineMesh(panelPipeline);
	Buffer vertexBuffer = getGraphicsMeshVertexBuffer(mesh);
	Buffer indexBuffer = getGraphicsMeshIndexBuffer(mesh);
	Shader* shaders = getGraphicsPipelineShaders(panelPipeline);
	Shader vertexShader = shaders[0];
	Shader fragmentShader = shaders[1];
	destroyGraphicsPipeline(panelPipeline);
	destroyGraphicsMesh(mesh);
	destroyBuffer(indexBuffer);
	destroyBuffer(vertexBuffer);
	destroyShader(fragmentShader);
	destroyShader(vertexShader);
}

inline static GraphicsPipeline createTextPipelineInstance(
	Logger logger,
	PackReader packReader,
	Framebuffer framebuffer)
{
	assert(logger);
	assert(packReader);
	assert(framebuffer);

	GraphicsAPI api = getGraphicsAPI();

	const void* vertexPath;
	const void* fragmentPath;

	if (api == VULKAN_GRAPHICS_API)
	{
		vertexPath = "shaders/vulkan/text.vert.spv";
		fragmentPath = "shaders/vulkan/text.frag.spv";
	}
	else if (api == OPENGL_GRAPHICS_API ||
			 api == OPENGL_ES_GRAPHICS_API)
	{
		vertexPath = "shaders/opengl/text.vert";
		fragmentPath = "shaders/opengl/text.frag";
	}
	else
	{
		abort();
	}

	Window window = getFramebufferWindow(framebuffer);

	Shader vertexShader = createShaderFromPack(
		packReader,
		vertexPath,
		window,
		VERTEX_SHADER_TYPE,
		logger);

	if (!vertexShader)
		return NULL;

	Shader fragmentShader = createShaderFromPack(
		packReader,
		fragmentPath,
		window,
		FRAGMENT_SHADER_TYPE,
		logger);

	if (!fragmentShader)
	{
		destroyShader(vertexShader);
		return NULL;
	}

	Sampler sampler;

	MpgxResult mpgxResult = createTextSampler(
		window,
		&sampler);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create text sampler. "
			"(error: %s)", mpgxResultToString(mpgxResult));
		destroyShader(fragmentShader);
		destroyShader(vertexShader);
		return NULL;
	}

	GraphicsPipeline pipeline;

	mpgxResult = createTextPipeline(
		framebuffer,
		vertexShader,
		fragmentShader,
		sampler,
		NULL,
		0,
		&pipeline);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create text pipeline. "
			"(error: %s)", mpgxResultToString(mpgxResult));
		destroySampler(sampler);
		destroyShader(fragmentShader);
		destroyShader(vertexShader);
		return NULL;
	}

	return pipeline;
}
inline static void destroyTextPipelineInstance(
	GraphicsPipeline textPipeline)
{
	if (!textPipeline)
		return;

	Shader* shaders = getGraphicsPipelineShaders(textPipeline);
	Shader vertexShader = shaders[0];
	Shader fragmentShader = shaders[1];
	Sampler sampler = getTextPipelineSampler(textPipeline);
	destroyGraphicsPipeline(textPipeline);
	destroySampler(sampler);
	destroyShader(fragmentShader);
	destroyShader(vertexShader);
}

inline static FontAtlas createFontAtlasInstance(
	Logger logger,
	PackReader packReader,
	GraphicsPipeline textPipeline,
	int fontSize)
{
	assert(logger);
	assert(packReader);
	assert(textPipeline);
	assert(fontSize > 0);

	Font regularMainFont = createFontFromPack(
		packReader,
		"fonts/jetbrains-mono-regular.ttf",
		0,
		logger);

	if (!regularMainFont)
		return NULL;

	Font regularFallbackFont = createFontFromPack(
		packReader,
		"fonts/noto-sans-regular.ttf",
		0,
		logger);

	if (!regularFallbackFont)
	{
		destroyFont(regularMainFont);
		return NULL;
	}

	Font boldMainFont = createFontFromPack(
		packReader,
		"fonts/jetbrains-mono-bold.ttf",
		0,
		logger);

	if (!boldMainFont)
	{
		destroyFont(regularFallbackFont);
		destroyFont(regularMainFont);
		return NULL;
	}

	Font boldFallbackFont = createFontFromPack(
		packReader,
		"fonts/noto-sans-bold.ttf",
		0,
		logger);

	if (!boldFallbackFont)
	{
		destroyFont(boldMainFont);
		destroyFont(regularFallbackFont);
		destroyFont(regularMainFont);
		return NULL;
	}

	Font italicMainFont = createFontFromPack(
		packReader,
		"fonts/jetbrains-mono-italic.ttf",
		0,
		logger);

	if (!italicMainFont)
	{
		destroyFont(boldFallbackFont);
		destroyFont(boldMainFont);
		destroyFont(regularFallbackFont);
		destroyFont(regularMainFont);
		return NULL;
	}

	Font italicFallbackFont = createFontFromPack(
		packReader,
		"fonts/noto-sans-italic.ttf",
		0,
		logger);

	if (!italicFallbackFont)
	{
		destroyFont(italicMainFont);
		destroyFont(boldFallbackFont);
		destroyFont(boldMainFont);
		destroyFont(regularFallbackFont);
		destroyFont(regularMainFont);
		return NULL;
	}

	Font boldItalicMainFont = createFontFromPack(
		packReader,
		"fonts/jetbrains-mono-bold-italic.ttf",
		0,
		logger);

	if (!boldItalicMainFont)
	{
		destroyFont(italicFallbackFont);
		destroyFont(italicMainFont);
		destroyFont(boldFallbackFont);
		destroyFont(boldMainFont);
		destroyFont(regularFallbackFont);
		destroyFont(regularMainFont);
		return NULL;
	}

	Font boldItalicFallbackFont = createFontFromPack(
		packReader,
		"fonts/noto-sans-bold-italic.ttf",
		0,
		logger);

	if (!boldItalicFallbackFont)
	{
		destroyFont(boldItalicMainFont);
		destroyFont(italicFallbackFont);
		destroyFont(italicMainFont);
		destroyFont(boldFallbackFont);
		destroyFont(boldMainFont);
		destroyFont(regularFallbackFont);
		destroyFont(regularMainFont);
		return NULL;
	}

	Font regularFonts[2] = {
		regularMainFont,
		regularFallbackFont
	};
	Font boldFonts[2] = {
		boldMainFont,
		boldFallbackFont
	};
	Font italicFonts[2] = {
		italicMainFont,
		italicFallbackFont
	};
	Font boldItalicFonts[2] = {
		boldItalicMainFont,
		boldItalicFallbackFont
	};

	FontAtlas fontAtlas;

	MpgxResult mpgxResult = createAsciiFontAtlas(
		textPipeline,
		regularFonts,
		boldFonts,
		italicFonts,
		boldItalicFonts,
		2,
		fontSize,
		logger,
		&fontAtlas);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create regular font atlas. "
			"(error: %s)", mpgxResultToString(mpgxResult));
		destroyFont(boldItalicFallbackFont);
		destroyFont(boldItalicMainFont);
		destroyFont(italicFallbackFont);
		destroyFont(italicMainFont);
		destroyFont(boldFallbackFont);
		destroyFont(boldMainFont);
		destroyFont(regularFallbackFont);
		destroyFont(regularMainFont);
		return NULL;
	}

	return fontAtlas;
}
inline static void destroyFontAtlasInstance(FontAtlas fontAtlas)
{
	if (!fontAtlas)
		return;

	Font* fonts = getFontAtlasRegularFonts(fontAtlas);
	Font regularMainFont = fonts[0];
	Font regularFallbackFont = fonts[1];
	fonts = getFontAtlasBoldFonts(fontAtlas);
	Font boldMainFont = fonts[0];
	Font boldFallbackFont = fonts[1];
	fonts = getFontAtlasItalicFonts(fontAtlas);
	Font italicMainFont = fonts[0];
	Font italicFallbackFont = fonts[1];
	fonts = getFontAtlasBoldItalicFonts(fontAtlas);
	Font boldItalicMainFont = fonts[0];
	Font boldItalicFallbackFont = fonts[1];
	destroyFontAtlas(fontAtlas);
	destroyFont(boldItalicFallbackFont);
	destroyFont(boldItalicMainFont);
	destroyFont(italicFallbackFont);
	destroyFont(italicMainFont);
	destroyFont(boldFallbackFont);
	destroyFont(boldMainFont);
	destroyFont(regularFallbackFont);
	destroyFont(regularMainFont);
}

inline static Window getProgramWindow(Program program)
{
	assert(program);
	return program->window;
}
static void onProgramUpdate(void* argument)
{
	assert(argument);
	Program program = (Program)argument;
	Window window = program->window;
	UserInterface ui = program->ui;
	Framebuffer framebuffer = getWindowFramebuffer(window);

	FramebufferClear clearValues[2];
	clearValues[0].color = zeroLinearColor;
	DepthStencilClear depthClear = { 1.0f, 0 };
	clearValues[1].depthStencil = depthClear;

	updateUserInterface(ui);
	updateTransformer(program->transformer);

	beginWindowRecord(window);
	beginFramebufferRender(
		framebuffer,
		clearValues,
		2);
	drawUserInterface(ui);
	endFramebufferRender(framebuffer);
	endWindowRecord(window);
}

inline static void destroyProgram(Program program)
{
	if (!program)
		return;

	destroyEditor(program->editor);
	destroyUserInterface(program->ui);
	destroyFontAtlasInstance(program->fontAtlas);
	destroyTextPipelineInstance(program->textPipeline);
	destroyPanelPipelineInstance(program->panelPipeline);
	destroyWindow(program->window);
	destroyTransformer(program->transformer);
	terminateText(program->logger);
	terminateGraphics();
	storeSettings(program->logger, program->settings);
	free(program);
}
inline static Program createProgram(
	Logger logger,
	ThreadPool threadPool)
{
	assert(logger);
	assert(threadPool);

	logMessage(logger, INFO_LOG_LEVEL,
		"Uran - Editor (v" URAN_VERSION_STRING ")");

	Program program = calloc(
		1, sizeof(Program_T));

	if (!program)
		return NULL;

	program->logger = logger;
	program->threadPool = threadPool;

	PackReader packReader;

	PackResult packResult = createFilePackReader(
		RESOURCES_FILE_PATH,
		0,
		&packReader);

	if (packResult != SUCCESS_PACK_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create pack reader. (error: %s)",
			packResultToString(packResult));
		destroyProgram(program);
		return NULL;
	}

	Settings* settings = &program->settings;
	loadSettings(logger, settings);

	MpgxResult mpgxResult = initializeGraphics(
		settings->graphicsAPI,
		ENGINE_NAME,
		URAN_VERSION_MAJOR,
		URAN_VERSION_MINOR,
		URAN_VERSION_PATCH,
		APPLICATION_NAME,
		URAN_VERSION_MAJOR,
		URAN_VERSION_MINOR,
		URAN_VERSION_PATCH);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to initialize graphics subsystems. "
			"(error: %s)", mpgxResultToString(mpgxResult));
		destroyPackReader(packReader);
		destroyProgram(program);
		return NULL;
	}

	if (!initializeText(logger))
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to initialize text subsystems.");
		destroyPackReader(packReader);
		destroyProgram(program);
		return NULL;
	}

	Transformer transformer = createTransformer(0, threadPool);

	if (!transformer)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create transformer.");
		destroyPackReader(packReader);
		destroyProgram(program);
		return NULL;
	}

	program->transformer = transformer;

	Window window;

	mpgxResult = createWindow(
		onProgramUpdate,
		program,
		false,
		true,
		false,
		NULL,
		&window);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create window. (error: %s)",
			mpgxResultToString(mpgxResult));
		destroyPackReader(packReader);
		destroyProgram(program);
		return NULL;
	}

	program->window = window;
	setWindowTitle(window, WINDOW_TITLE);

	Framebuffer framebuffer = getWindowFramebuffer(window);

	GraphicsPipeline panelPipeline = createPanelPipelineInstance(
		logger,
		packReader,
		framebuffer);

	if (!panelPipeline)
	{
		destroyPackReader(packReader);
		destroyProgram(program);
		return NULL;
	}

	program->panelPipeline = panelPipeline;

	GraphicsPipeline textPipeline = createTextPipelineInstance(
		logger,
		packReader,
		framebuffer);

	if (!textPipeline)
	{
		destroyPackReader(packReader);
		destroyProgram(program);
		return NULL;
	}

	program->textPipeline = textPipeline;

	FontAtlas fontAtlas = createFontAtlasInstance(
		logger,
		packReader,
		textPipeline,
		48);

	if (!fontAtlas)
	{
		destroyPackReader(packReader);
		destroyProgram(program);
		return NULL;
	}

	program->fontAtlas = fontAtlas;

	UserInterface ui;

	mpgxResult = createUserInterface(
		transformer,
		panelPipeline,
		textPipeline,
		fontAtlas,
		1.0f,
		0,
		threadPool,
		&ui);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create UI. (error: %s)",
			mpgxResultToString(mpgxResult));
		destroyPackReader(packReader);
		destroyProgram(program);
		return NULL;
	}

	program->ui = ui;

	Editor editor = createEditor(
		logger,
		window,
		ui);

	if (!editor)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create editor.");
		destroyProgram(program);
		return NULL;
	}

	program->editor = editor;
	return program;
}
