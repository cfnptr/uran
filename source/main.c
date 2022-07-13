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

#include "uran/defines.h"
#include "uran/entry.h"
#include "uran/editor.h"
#include "uran/shader_data.h"
#include "uran/primitives/square_primitive.h"

#include "mpmt/common.h"
#include "conf/reader.h"
#include "conf/writer.h"

#include <stdio.h>
#include <string.h>

#if __linux__ || __APPLE__
#include <sys/utsname.h>
#endif

#define LOG_DIRECTORY_PATH "logs"
#define SETTINGS_FILE_PATH "editor-settings.txt"
#define RESOURCES_FILE_PATH "editor-resources.pack"
#define ENGINE_NAME "Uran"
#define APPLICATION_NAME "Editor"
#define WINDOW_TITLE "Uran Editor"
#define FONT_ATLAS_COUNT 2

typedef struct Settings
{
	GraphicsAPI graphicsAPI;
	cmmt_float_t uiScale;
	bool isAutoGraphicsAPI;
} Settings;
typedef struct Program_T
{
	Logger logger;
	ThreadPool threadPool;
	Transformer transformer;
	Window window;
	GraphicsPipeline panelPipeline;
	GraphicsPipeline textPipeline;
	FontAtlas fontAtlases[FONT_ATLAS_COUNT];
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
			"Failed to load \"settings.txt\" file. "
			"(error: %s at line %zu)",
			confResultToString(confResult), errorLine);

		settings.graphicsAPI = VULKAN_GRAPHICS_API;
		settings.uiScale = (cmmt_float_t)1.0;
		settings.isAutoGraphicsAPI = true;
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
		settings.isAutoGraphicsAPI = true;
	}
	else
	{
		if (strcmp(stringValue, "auto") == 0)
		{
			settings.graphicsAPI = VULKAN_GRAPHICS_API;
			settings.isAutoGraphicsAPI = true;
		}
		else if (strcmp(stringValue, "vulkan") == 0)
		{
			settings.graphicsAPI = VULKAN_GRAPHICS_API;
			settings.isAutoGraphicsAPI = false;
		}
		else if (strcmp(stringValue, "opengl") == 0)
		{
			settings.graphicsAPI = OPENGL_GRAPHICS_API;
			settings.isAutoGraphicsAPI = false;
		}
		else
		{
			logMessage(logger, WARN_LOG_LEVEL,
				"Unknown \"graphicsAPI\" settings value.");
			settings.graphicsAPI = VULKAN_GRAPHICS_API;
		}
	}

	double floatingValue;

	result = getConfReaderFloating(confReader,
		"uiScale", &floatingValue);

	if (!result)
	{
		logMessage(logger, WARN_LOG_LEVEL,
			"Failed to read \"uiScale\" settings value.");
		settings.uiScale = (cmmt_float_t)1.0;
	}
	else
	{
		if (floatingValue <= 0.0 || (int)(56 * floatingValue) % 2 != 0)
		{
			logMessage(logger, WARN_LOG_LEVEL,
				"Invalid \"uiScale\" settings value.");
			settings.uiScale = (cmmt_float_t)1.0;
		}
		else
		{
			settings.uiScale = (cmmt_float_t)floatingValue;
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
		"=============  Uran Editor - Settings (v"
		URAN_VERSION_STRING ")  ==============");
	result &= writeConfComment(confWriter,
		"Copyright (c) 2020-2022 Nikita Fediuchin. All rights reserved.");
	result &= writeConfComment(confWriter,
		"Licensed under the Apache License, Version 2.0 (\"AS IS\" BASIS)");
	result &= writeConfComment(confWriter,
		"==============================================================");
	result &= writeConfNewLine(confWriter);

	const char* stringValue;

	if (settings.isAutoGraphicsAPI == true)
		stringValue = "auto";
	else if (settings.graphicsAPI == VULKAN_GRAPHICS_API)
		stringValue = "vulkan";
	else if (settings.graphicsAPI == OPENGL_GRAPHICS_API)
		stringValue = "opengl";
	else
		abort();

	result &= writeConfComment(confWriter,
		"Graphics rendering backend.");
	result &= writeConfComment(confWriter,
		"(auto, vulkan, opengl)");
	result &= writeConfString(confWriter,
		"graphicsAPI", stringValue, 0);
	result &= writeConfNewLine(confWriter);

	result &= writeConfComment(confWriter,
		"User interface scaling factor.");
	result &= writeConfFloating(confWriter,
		"uiScale", settings.uiScale);

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
	else if (api == OPENGL_GRAPHICS_API)
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
		true,
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
	else if (api == OPENGL_GRAPHICS_API)
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
		true,
		1,
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

inline static uint32_t processFontSize(
	cmmt_float_t scale,
	uint32_t fontSize)
{
	fontSize = (uint32_t)((cmmt_float_t)fontSize * scale);

	if (fontSize % 2 != 0)
		fontSize += 1;

	return fontSize;
}
inline static bool createFontAtlasArray(
	Logger logger,
	PackReader packReader,
	GraphicsPipeline textPipeline,
	cmmt_float_t scale,
	FontAtlas* fontAtlases)
{
	assert(logger);
	assert(packReader);
	assert(textPipeline);
	assert(fontAtlases);

	Font regularMainFont = createFontFromPack(
		packReader,
		"fonts/jetbrains-mono-regular.ttf",
		0,
		logger);

	if (!regularMainFont)
		return false;

	Font regularFallbackFont = createFontFromPack(
		packReader,
		"fonts/noto-sans-regular.ttf",
		0,
		logger);

	if (!regularFallbackFont)
	{
		destroyFont(regularMainFont);
		return false;
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
		return false;
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
		return false;
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
		return false;
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
		return false;
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
		return false;
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
		return false;
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

	scale *= getPlatformScale(
		getGraphicsPipelineFramebuffer(
		textPipeline));

	FontAtlas fontAtlas;

	MpgxResult mpgxResult = createAsciiFontAtlas(
		textPipeline,
		regularFonts,
		boldFonts,
		italicFonts,
		boldItalicFonts,
		2,
		processFontSize(scale, 14),
		logger,
		&fontAtlas);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create font atlas. (error: %s)",
			mpgxResultToString(mpgxResult));
		goto DESTROY_FONTS;
	}

	fontAtlases[0] = fontAtlas;

	mpgxResult = createAsciiFontAtlas(
		textPipeline,
		regularFonts,
		boldFonts,
		italicFonts,
		boldItalicFonts,
		2,
		processFontSize(scale, 16),
		logger,
		&fontAtlas);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create font atlas. (error: %s)",
			mpgxResultToString(mpgxResult));
		destroyFontAtlas(fontAtlases[0]);
		goto DESTROY_FONTS;
	}

	fontAtlases[1] = fontAtlas;
	return true;

DESTROY_FONTS:
	destroyFont(boldItalicFallbackFont);
	destroyFont(boldItalicMainFont);
	destroyFont(italicFallbackFont);
	destroyFont(italicMainFont);
	destroyFont(boldFallbackFont);
	destroyFont(boldMainFont);
	destroyFont(regularFallbackFont);
	destroyFont(regularMainFont);
	return false;
}
inline static void destroyFontAtlasArray(FontAtlas* fontAtlases)
{
	if (!fontAtlases[0])
		return;

	FontAtlas fontAtlas = fontAtlases[0];
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

	for (size_t i = 0; i < FONT_ATLAS_COUNT; i++)
		destroyFontAtlas(fontAtlases[i]);

	destroyFont(boldItalicFallbackFont);
	destroyFont(boldItalicMainFont);
	destroyFont(italicFallbackFont);
	destroyFont(italicMainFont);
	destroyFont(boldFallbackFont);
	destroyFont(boldMainFont);
	destroyFont(regularFallbackFont);
	destroyFont(regularMainFont);
}

inline static void onDraw(Program program)
{
	assert(program);
	Window window = program->window;
	UserInterface ui = program->ui;
	Framebuffer framebuffer = getWindowFramebuffer(window);
	GraphicsRendererResult result = createGraphicsRendererResult();
	GraphicsRendererResult tmpResult;
	FramebufferClear clearValues[2];
	clearValues[0].color = zeroLinearColor;
	DepthStencilClear depthClear = { 1.0f, 0 };
	clearValues[1].depthStencil = depthClear;

	MpgxResult mpgxResult = beginWindowRecord(window);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(program->logger, FATAL_LOG_LEVEL,
			"Failed to begin window record. (error: %s)",
		mpgxResultToString(mpgxResult));
		abort();
	}

	beginFramebufferRender(
		framebuffer,
		clearValues,
	2);

	tmpResult = drawUserInterface(ui);
	result = addGraphicsRendererResult(result, tmpResult);

	endFramebufferRender(framebuffer);

	Editor editor = program->editor;
	setEditorRendererResult(editor, result);
	postUpdateEditor(editor);
	endWindowRecord(window);
}
static void onProgramUpdate(void* argument)
{
	assert(argument);
	Program program = (Program)argument;
	updateUserInterface(program->ui);
	updateTransformer(program->transformer);
	onDraw(program);
}

inline static void destroyProgram(Program program)
{
	if (!program)
		return;

	destroyEditor(program->editor);
	destroyUserInterface(program->ui);
	destroyFontAtlasArray(program->fontAtlases);
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

	Program program = calloc(
		1, sizeof(Program_T));

	if (!program)
		return NULL;

	program->logger = logger;
	program->threadPool = threadPool;

	Settings* settings = &program->settings;
	loadSettings(logger, settings);

	PackReader packReader;

	PackResult packResult = createFilePackReader(
		RESOURCES_FILE_PATH,
		0,
		false,
		&packReader);

	if (packResult != SUCCESS_PACK_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create pack reader. (error: %s)",
			packResultToString(packResult));
		destroyProgram(program);
		return NULL;
	}

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
		if (settings->isAutoGraphicsAPI)
		{
			logMessage(logger, WARN_LOG_LEVEL,
				"Failed to initialize Vulkan graphics subsystems, "
				"trying OpenGL... (error: %s)",
				mpgxResultToString(mpgxResult));

			mpgxResult = initializeGraphics(
				OPENGL_GRAPHICS_API,
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
		}
		else
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to initialize graphics subsystems. "
				"(error: %s)", mpgxResultToString(mpgxResult));
			destroyPackReader(packReader);
			destroyProgram(program);
			return NULL;
		}
	}

	logMessage(logger, INFO_LOG_LEVEL,
		"Graphics API: %s.",
		graphicsApiToString(getGraphicsAPI()));

	if (!initializeText(logger))
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to initialize text subsystems.");
		destroyPackReader(packReader);
		destroyProgram(program);
		return NULL;
	}

	Transformer transformer = createTransformer(1, threadPool);

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

	logMessage(logger, INFO_LOG_LEVEL,
		"GPU: %s.",
		getWindowGpuName(window));
	logMessage(logger, INFO_LOG_LEVEL,
		"GPU driver: %s.",
		getWindowGpuDriver(window));

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

	FontAtlas* fontAtlases = program->fontAtlases;

	bool result = createFontAtlasArray(
		logger,
		packReader,
		textPipeline,
		settings->uiScale,
		fontAtlases);

	if (!result)
	{
		destroyPackReader(packReader);
		destroyProgram(program);
		return NULL;
	}

	UserInterface ui;

	mpgxResult = createUserInterface(
		transformer,
		panelPipeline,
		textPipeline,
		fontAtlases,
		FONT_ATLAS_COUNT,
		settings->uiScale,
		1,
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

	destroyPackReader(packReader);
	showWindow(window);
	return program;
}

URAN_MAIN_FUNCTION
{
#ifndef NDEBUG
	LogLevel logLevel = ALL_LOG_LEVEL;
	bool logToStdout = true;
#else
	LogLevel logLevel = INFO_LOG_LEVEL;
	bool logToStdout = false;
#endif

	Logger logger;

	LogyResult logyResult = createLogger(
		LOG_DIRECTORY_PATH,
		logLevel,
		logToStdout,
		0.0,
		false,
		&logger);

	if (logyResult != SUCCESS_LOGY_RESULT)
	{
		fprintf(stderr,
			"Failed to create logger. (error: %s)\n",
			logyResultToString(logyResult));
		return EXIT_FAILURE;
	}

	logMessage(logger, INFO_LOG_LEVEL,
		"Uran - Editor (v" URAN_VERSION_STRING ")");

#if __linux__ || __APPLE__
	struct utsname unameData;
	int result = uname(&unameData);

	logMessage(logger, INFO_LOG_LEVEL, "OS: %s %s %s %s.",
		unameData.sysname, unameData.release,
		unameData.version, unameData.machine);
#elif _WIN32
	logMessage(logger, INFO_LOG_LEVEL, "OS: Windows.");
#else
#error Unknown operating system
#endif

	int cpuCount = getCpuCount();

	logMessage(logger, INFO_LOG_LEVEL,
		"CPU: %s.", getCpuName());
	logMessage(logger, INFO_LOG_LEVEL,
		"CPU count: %d.", cpuCount);

	ThreadPool threadPool = createThreadPool(
		cpuCount,
		cpuCount,
		STACK_TASK_ORDER_TYPE);

	if (!threadPool)
	{
		logMessage(logger, FATAL_LOG_LEVEL,
			"Failed to create thread pool.");
		destroyLogger(logger);
		return EXIT_FAILURE;
	}

	Program program = createProgram(
		logger,
		threadPool);

	if (!program)
	{
		logMessage(logger, FATAL_LOG_LEVEL,
			"Failed to create program.");
		destroyThreadPool(threadPool);
		destroyLogger(logger);
		return EXIT_FAILURE;
	}

	joinWindow(program->window);

	destroyProgram(program);
	destroyThreadPool(threadPool);
	destroyLogger(logger);
	return EXIT_SUCCESS;
}
