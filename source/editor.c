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

#include "uran/editor.h"
#include "uran/shader_data.h"
#include "uran/primitives/square_primitive.h"

#include "conf/reader.h"
#include "conf/writer.h"
#include "cmmt/angle.h"

#include <stdio.h>
#include <string.h>

#define EDITOR_SETTINGS_FILE_PATH "editor-settings.txt"
#define EDITOR_RESOURCES_FILE_PATH "editor-resources.pack"
#define ENGINE_NAME "Uran"
#define APPLICATION_NAME "Editor"
#define WINDOW_TITLE "Uran Editor"

typedef struct BaseWindow_T
{
	InterfaceElement window;
	InterfaceElement closeButton;
} BaseWindow_T;

typedef BaseWindow_T* BaseWindow;

typedef struct StatsWindow_T
{
	Logger logger;
	Window window;
	BaseWindow base;
	InterfaceElement label;
	double lastUpdateTime;
	GraphicsRendererResult rendererResult;
} StatsWindow_T;

typedef StatsWindow_T* StatsWindow;

typedef struct MenuBar_T
{
	Window window;
	StatsWindow statsWindow;
	InterfaceElement panel;
	InterfaceElement statsButton;
} MenuBar_T;

typedef MenuBar_T* MenuBar;

typedef struct Settings
{
	GraphicsAPI graphicsAPI;
} Settings;
struct Editor_T
{
	Logger logger;
	Transformer transformer;
	Window window;
	GraphicsPipeline panelPipeline;
	GraphicsPipeline textPipeline;
	FontAtlas fontAtlas;
	UserInterface ui;
	StatsWindow statsWindow;
	MenuBar menuBar;
	Settings settings;
};

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
		EDITOR_SETTINGS_FILE_PATH,
		&confReader,
		&errorLine);

	if (confResult != SUCCESS_CONF_RESULT)
	{
		logMessage(logger, WARN_LOG_LEVEL,
			"Failed to create settings reader. (error: %s at line %zu)",
			confResultToString(confResult), errorLine);

		settings.graphicsAPI = VULKAN_GRAPHICS_API;
		*_settings = settings;
		return;
	}

	const char* stringValue;

	bool result = getConfReaderString(confReader,
		"graphicsAPI", &stringValue);

	if (!result)
	{
		logMessage(logger, WARN_LOG_LEVEL,
			"Failed to read \"graphicsAPI\" settings value.");
		settings.graphicsAPI = VULKAN_GRAPHICS_API;
	}
	else
	{
		if (strcmp(stringValue, "vulkan") == 0)
		{
			settings.graphicsAPI = VULKAN_GRAPHICS_API;
		}
		else if (strcmp(stringValue, "opengl") == 0)
		{
			settings.graphicsAPI = OPENGL_GRAPHICS_API;
		}
		else if (strcmp(stringValue, "opengles") == 0)
		{
			settings.graphicsAPI = OPENGL_ES_GRAPHICS_API;
		}
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
		EDITOR_SETTINGS_FILE_PATH,
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

static void onBaseWindowEnter(InterfaceElement element)
{
	assert(element);
	BaseWindow baseWindow = getUiWindowHandle(element);
	Transform transform = getGraphicsRenderTransform(
		getUiButtonTextRender(baseWindow->closeButton));
	setTransformActive(transform, true);
}
static void onBaseWindowExit(InterfaceElement element)
{
	assert(element);
	BaseWindow baseWindow = getUiWindowHandle(element);
	Transform transform = getGraphicsRenderTransform(
		getUiButtonTextRender(baseWindow->closeButton));
	setTransformActive(transform, false);
}
static void onBaseWindowCloseEnter(InterfaceElement element)
{
	assert(element);
	BaseWindow baseWindow = getUiButtonHandle(element);
	Transform transform = getGraphicsRenderTransform(
		getUiButtonTextRender(baseWindow->closeButton));
	setTransformActive(transform, true);
}
static void onBaseWindowCloseExit(InterfaceElement element)
{
	assert(element);
	BaseWindow baseWindow = getUiButtonHandle(element);
	Transform transform = getGraphicsRenderTransform(
		getUiButtonTextRender(baseWindow->closeButton));
	setTransformActive(transform, false);
}
static void onBaseWindowCloseRelease(InterfaceElement element)
{
	assert(element);
	BaseWindow baseWindow = getUiButtonHandle(element);
	Transform transform = getInterfaceElementTransform(
		baseWindow->window);
	setTransformActive(transform, false);
}
inline static void destroyBaseWindow(BaseWindow baseWindow)
{
	if (!baseWindow)
		return;

	destroyInterfaceElement(baseWindow->closeButton);
	destroyInterfaceElement(baseWindow->window);
	free(baseWindow);
}
inline static BaseWindow createBaseWindow(
	UserInterface ui,
	const uint32_t* title,
	size_t titleLength,
	Vec3F position,
	Vec2F scale,
	Logger logger)
{
	assert(ui);
	assert(title);
	assert(titleLength > 0);
	assert(scale.x > 0.0f);
	assert(scale.y > 0.0f);
	assert(logger);

	BaseWindow baseWindow = calloc(
		1, sizeof(BaseWindow_T));

	if (!baseWindow)
		return NULL;

	InterfaceElementEvents events = emptyInterfaceElementEvents;
	events.onEnter = onBaseWindowEnter;
	events.onExit = onBaseWindowExit;

	InterfaceElement window;

	MpgxResult mpgxResult = createUiWindow32(ui,
		title,
		titleLength,
		CENTER_ALIGNMENT_TYPE,
		position,
		scale,
		DEFAULT_UI_TEXT_COLOR,
		NULL,
		&events,
		baseWindow,
		false,
		&window);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create UI window. (error: %s)",
			mpgxResultToString(mpgxResult));
		destroyBaseWindow(baseWindow);
		return NULL;
	}

	baseWindow->window = window;

	Transform windowTransform =
		getInterfaceElementTransform(window);

	const uint32_t text[] = { '+' };

	events.onEnter = onBaseWindowCloseEnter;
	events.onExit = onBaseWindowCloseExit;
	events.onRelease = onBaseWindowCloseRelease;

	InterfaceElement closeButton;

	mpgxResult = createUiButton32(ui,
		text,
		sizeof(text) / sizeof(uint32_t),
		LEFT_ALIGNMENT_TYPE,
		vec3F(
			(cmmt_float_t)12.0,
			(cmmt_float_t)0.0,
			(cmmt_float_t)-0.01),
		valVec2F((cmmt_float_t)16.0),
		DEFAULT_UI_TEXT_COLOR,
		windowTransform,
		&events,
		baseWindow,
		true,
		true,
		&closeButton);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create UI button. (error: %s)",
			mpgxResultToString(mpgxResult));
		destroyBaseWindow(baseWindow);
		return NULL;
	}

	Transform crossTransform = getGraphicsRenderTransform(
		getUiButtonTextRender(closeButton));
	setTransformScale(
		crossTransform,
		vec3F(
			(cmmt_float_t)16.0,
			(cmmt_float_t)16.0,
			(cmmt_float_t)0.0));
	setTransformRotationType(
		crossTransform,
		SPIN_ROTATION_TYPE);
	setTransformEulerAngles(
		crossTransform,
		vec3F(
			(cmmt_float_t)0.0,
			(cmmt_float_t)0.0,
			degToRad((cmmt_float_t)45.0)));
	setTransformActive(
		crossTransform,
		false);

	baseWindow->closeButton = closeButton;
	return baseWindow;
}

static void onStatsLabelUpdate(InterfaceElement element)
{
	assert(element);
	StatsWindow statsWindow = getUiLabelHandle(element);
	double updateTime = getWindowUpdateTime(statsWindow->window);

	if (updateTime < statsWindow->lastUpdateTime)
		return;

	double deltaTime = getWindowDeltaTime(statsWindow->window);
	Text text = getTextRenderText(getUiLabelRender(element));
	GraphicsRendererResult rendererResult = statsWindow->rendererResult;

	char bufferUTF8[256];
	uint32_t bufferUTF32[256];

	size_t count = snprintf(
		bufferUTF8,
		256,
		"<b>FPS</b>: %d (<i>%dms</i>)\n"
		"<b>Draw count</b>: %zu\n"
		"<b>Polygon count</b>: %zu\n"
		"<b>Pass count</b>: %zu",
		(int)(1.0 / deltaTime),
		(int)(deltaTime * 1000.0),
		rendererResult.drawCount,
		rendererResult.indexCount / 3,
		rendererResult.passCount);
	count = stringUTF8toUTF32(
		bufferUTF8,
		count,
		bufferUTF32);

	MpgxResult mpgxResult = bakeText32(
		text,
		bufferUTF32,
		count,
		LEFT_TOP_ALIGNMENT_TYPE,
		DEFAULT_UI_TEXT_COLOR,
		true,
		false,
		false);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(statsWindow->logger, FATAL_LOG_LEVEL,
			"Failed to bake stats text. (error: %s)",
			mpgxResultToString(mpgxResult));
		abort();
	}

	statsWindow->lastUpdateTime = updateTime + 0.1;
}
inline static void destroyStatsWindow(StatsWindow statsWindow)
{
	if (!statsWindow)
		return;

	destroyInterfaceElement(statsWindow->label);
	destroyBaseWindow(statsWindow->base);
	free(statsWindow);
}
inline static StatsWindow createStatsWindow(
	UserInterface ui,
	Logger logger,
	Window window)
{
	assert(ui);
	assert(logger);
	assert(window);

	StatsWindow statsWindow = calloc(
		1, sizeof(StatsWindow_T));

	if (!statsWindow)
		return NULL;

	const uint32_t windowTitle[] = {
		'S', 't', 'a', 't', 's',
	};

	BaseWindow base = createBaseWindow(ui,
		windowTitle,
		sizeof(windowTitle) / sizeof(uint32_t),
		zeroVec3F,
		vec2F(
			(cmmt_float_t)256.0,
			(cmmt_float_t)128.0),
		logger);

	if (!base)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create base window.");
		destroyStatsWindow(statsWindow);
		return NULL;
	}

	statsWindow->logger = logger;
	statsWindow->window = window;
	statsWindow->base = base;

	Transform windowTransform =
		getInterfaceElementTransform(base->window);

	const uint32_t labelText[] = {
		'L', 'o', 'a', 'd', 'i', 'n', 'g',
	};

	InterfaceElementEvents events = emptyInterfaceElementEvents;
	events.onUpdate = onStatsLabelUpdate;

	InterfaceElement label;

	MpgxResult mpgxResult = createUiLabel32(ui,
		labelText,
		sizeof(labelText) / sizeof(uint32_t),
		LEFT_TOP_ALIGNMENT_TYPE,
		vec3F(
			(cmmt_float_t)8.0,
			(cmmt_float_t)-32.0,
			(cmmt_float_t)-0.001),
		(cmmt_float_t)DEFAULT_UI_TEXT_HEIGHT,
		DEFAULT_UI_TEXT_COLOR,
		true,
		false,
		true,
		false,
		windowTransform,
		&events,
		statsWindow,
		true,
		&label);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create UI label. (error: %s)",
			mpgxResultToString(mpgxResult));
		destroyStatsWindow(statsWindow);
		return NULL;
	}

	statsWindow->label = label;
	return statsWindow;
}

inline static void onMenuBarUpdate(InterfaceElement element)
{
	assert(element);
	MenuBar menuBar = getUiPanelHandle(element);
	Transform transform = getInterfaceElementTransform(menuBar->panel);
	Vec2I windowSize = getWindowSize(menuBar->window);
	Vec3F scale = vec3F(
		(cmmt_float_t)windowSize.x,
		(cmmt_float_t)24.0,
		(cmmt_float_t)1.0);
	setTransformScale(transform, scale);
}
inline static void onMenuBarStatsRelease(InterfaceElement element)
{
	assert(element);
	MenuBar menuBar = getUiButtonHandle(element);
	InterfaceElement window = menuBar->statsWindow->base->window;
	Transform transform = getInterfaceElementTransform(window);
	setInterfaceElementPosition(window,
		vec3F(
			(cmmt_float_t)384.0,
			(cmmt_float_t)-128.0,
			(cmmt_float_t)0.1));
	setTransformActive(transform, true);
}
inline static void destroyMenuBar(MenuBar menuBar)
{
	if (!menuBar)
		return;

	destroyInterfaceElement(menuBar->statsButton);
	destroyInterfaceElement(menuBar->panel);
	free(menuBar);
}
inline static MenuBar createMenuBar(
	UserInterface ui,
	Logger logger,
	Window window,
	StatsWindow statsWindow)
{
	assert(ui);
	assert(logger);
	assert(window);
	assert(statsWindow);

	MenuBar menuBar = calloc(1,
		sizeof(MenuBar_T));

	if (!menuBar)
		return NULL;

	menuBar->window = window;
	menuBar->statsWindow = statsWindow;

	InterfaceElementEvents events = emptyInterfaceElementEvents;
	events.onUpdate = onMenuBarUpdate;

	InterfaceElement element;

	MpgxResult mpgxResult = createUiPanel(ui,
		TOP_ALIGNMENT_TYPE,
		vec3F(
			(cmmt_float_t)0.0,
			(cmmt_float_t)-12.0,
			(cmmt_float_t)-0.1),
		vec2F(
			(cmmt_float_t)defaultWindowSize.x,
			(cmmt_float_t)24.0),
		NULL,
		&events,
		menuBar,
		true,
		&element);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create UI panel. (error: %s)",
			mpgxResultToString(mpgxResult));
		destroyMenuBar(menuBar);
		return NULL;
	}

	setPanelRenderColor(getUiPanelRender(element),
		srgbToLinearColor(DEFAULT_UI_ENABLED_BUTTON_COLOR));

	menuBar->panel = element;

	Transform panelTransform =
		getInterfaceElementTransform(element);

	const uint32_t statsText[] = {
		'S', 't', 'a', 't', 's',
	};
	events.onUpdate = NULL;
	events.onRelease = onMenuBarStatsRelease;

	mpgxResult = createUiButton32(ui,
		statsText,
		sizeof(statsText) / sizeof(uint32_t),
		LEFT_ALIGNMENT_TYPE,
		vec3F(
			(cmmt_float_t)32.0,
			(cmmt_float_t)0.0,
			(cmmt_float_t)-0.01),
		vec2F(
			(cmmt_float_t)64.0,
			(cmmt_float_t)24.0),
		DEFAULT_UI_TEXT_COLOR,
		panelTransform,
		&events,
		menuBar,
		true,
		true,
		&element);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create UI button. (error: %s)",
			mpgxResultToString(mpgxResult));
		destroyMenuBar(menuBar);
		return NULL;
	}

	menuBar->statsButton = element;
	return menuBar;
}

Editor createEditor(
	Logger logger,
	ThreadPool threadPool,
	OnWindowUpdate onUpdate,
	void* updateArgument)
{
	assert(logger);
	assert(threadPool);
	assert(onUpdate);
	assert(updateArgument);

	Editor editorInstance = calloc(
		1, sizeof(Editor_T));

	if (!editorInstance)
		return NULL;

	editorInstance->logger = logger;

	PackReader packReader;

	PackResult packResult = createFilePackReader(
		EDITOR_RESOURCES_FILE_PATH,
		0,
		&packReader);

	if (packResult != SUCCESS_PACK_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create pack reader. (error: %s)",
			packResultToString(packResult));
		destroyEditor(editorInstance);
		return NULL;
	}

	Settings* settings = &editorInstance->settings;
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
			"Failed to initialize graphics subsystem. "
			"(error: %s)", mpgxResultToString(mpgxResult));
		destroyEditor(editorInstance);
		destroyPackReader(packReader);
		return NULL;
	}

	if (!initializeText(logger))
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to initialize text subsystem.");
		destroyEditor(editorInstance);
		destroyPackReader(packReader);
		return NULL;
	}

	Transformer transformer = createTransformer(0, threadPool);

	if (!transformer)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create transformer.");
		destroyEditor(editorInstance);
		destroyPackReader(packReader);
		return NULL;
	}

	editorInstance->transformer = transformer;

	Window window;

	mpgxResult = createWindow(
		onUpdate,
		updateArgument,
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
		destroyEditor(editorInstance);
		destroyPackReader(packReader);
		return NULL;
	}

	editorInstance->window = window;
	setWindowTitle(window, WINDOW_TITLE);

	Framebuffer framebuffer = getWindowFramebuffer(window);

	GraphicsPipeline panelPipeline = createPanelPipelineInstance(
		logger,
		packReader,
		framebuffer);

	if (!panelPipeline)
	{
		destroyEditor(editorInstance);
		destroyPackReader(packReader);
		return NULL;
	}

	editorInstance->panelPipeline = panelPipeline;

	GraphicsPipeline textPipeline = createTextPipelineInstance(
		logger,
		packReader,
		framebuffer);

	if (!textPipeline)
	{
		destroyEditor(editorInstance);
		destroyPackReader(packReader);
		return NULL;
	}

	editorInstance->textPipeline = textPipeline;

	FontAtlas fontAtlas = createFontAtlasInstance(
		logger,
		packReader,
		textPipeline,
		48);

	if (!fontAtlas)
	{
		destroyEditor(editorInstance);
		destroyPackReader(packReader);
		return NULL;
	}

	editorInstance->fontAtlas = fontAtlas;

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
		destroyEditor(editorInstance);
		destroyPackReader(packReader);
		return NULL;
	}

	editorInstance->ui = ui;

	StatsWindow statsWindow = createStatsWindow(
		ui,
		logger,
		window);

	if (!statsWindow)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create stats window.");
		destroyEditor(editorInstance);
		destroyPackReader(packReader);
		return NULL;
	}

	editorInstance->statsWindow = statsWindow;

	MenuBar menuBar = createMenuBar(
		ui,
		logger,
		window,
		statsWindow);

	if (!menuBar)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create menu bar.");
		destroyEditor(editorInstance);
		destroyPackReader(packReader);
		return NULL;
	}

	editorInstance->menuBar = menuBar;

	destroyPackReader(packReader);
	return editorInstance;
}
void destroyEditor(Editor editor)
{
	if (!editor)
		return;

	destroyMenuBar(editor->menuBar);
	destroyStatsWindow(editor->statsWindow);
	destroyUserInterface(editor->ui);
	destroyFontAtlasInstance(editor->fontAtlas);
	destroyTextPipelineInstance(editor->textPipeline);
	destroyPanelPipelineInstance(editor->panelPipeline);
	destroyWindow(editor->window);
	destroyTransformer(editor->transformer);
	terminateText(editor->logger);
	terminateGraphics();
	storeSettings(editor->logger, editor->settings);
	free(editor);
}

Transformer getEditorTransformer(Editor editor)
{
	assert(editor);
	return editor->transformer;
}
Window getEditorWindow(Editor editor)
{
	assert(editor);
	return editor->window;
}
void setEditorRendererResult(
	Editor editor,
	GraphicsRendererResult result)
{
	assert(editor);
	editor->statsWindow->rendererResult = result;
}

void updateEditor(Editor editor)
{
	assert(editor);
	updateUserInterface(editor->ui);
	updateTransformer(editor->transformer);
}
void renderEditor(Editor editor)
{
	assert(editor);

	Framebuffer framebuffer = getWindowFramebuffer(
		editor->window);

	FramebufferClear clearValues[2];
	clearValues[0].color = zeroLinearColor;
	DepthStencilClear depthClear = { 1.0f, 0 };
	clearValues[1].depthStencil = depthClear;

	beginFramebufferRender(
		framebuffer,
		clearValues,
		2);
	drawUserInterface(editor->ui);
	endFramebufferRender(framebuffer);
}
