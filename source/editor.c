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

#include "conf/reader.h"
#include "conf/writer.h"
#include "cmmt/angle.h"

#include <string.h>

#define EDITOR_SETTINGS_FILE_PATH "editor-settings.txt"
#define EDITOR_RESOURCES_FILE_PATH "editor-resources.pack"
#define ENGINE_NAME "Uran"
#define APPLICATION_NAME "Editor"
#define WINDOW_TITLE "Uran Editor"

/*#define INPUT_PANEL_COLOR srgbToLinearColor(srgbColor(32, 32, 32, 255))
#define FOCUSED_INPUT_COLOR srgbToLinearColor(srgbColor(128, 128, 128, 255))
#define INCORRECT_INPUT_COLOR srgbToLinearColor(srgbColor(192, 32, 32, 255))
#define DEFAULT_TEXT_COLOR srgbToLinearColor(srgbColor(240, 240, 240, 255))
#define PLACEHOLDER_TEXT_COLOR srgbToLinearColor(srgbColor(144, 144, 144, 255))
#define CHECKBOX_CHECK_COLOR srgbToLinearColor(srgbColor(128, 128, 128, 255))*/

typedef struct BaseWindow_T
{
	InterfaceElement window;
	InterfaceElement closeButton;
} BaseWindow_T;

typedef BaseWindow_T* BaseWindow;

typedef struct MenuWindow_T
{
	BaseWindow base;
	InterfaceElement versionLabel;
} MenuWindow_T;

typedef MenuWindow_T* MenuWindow;

typedef struct Settings
{
	GraphicsAPI graphicsAPI;
} Settings;
struct Editor_T
{
	Logger logger;
	Transformer transformer;
	Window window;
	GraphicsPipeline spritePipeline;
	GraphicsPipeline textPipeline;
	FontAtlas fontAtlas;
	UserInterface ui;
	MenuWindow menuWindow;
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
		if (strcmp(stringValue, "Vulkan") == 0)
		{
			settings.graphicsAPI = VULKAN_GRAPHICS_API;
		}
		else if (strcmp(stringValue, "OpenGL") == 0)
		{
			settings.graphicsAPI = OPENGL_GRAPHICS_API;
		}
		else if (strcmp(stringValue, "OpenGLES") == 0)
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
		stringValue = "Vulkan";
	else if (settings.graphicsAPI == OPENGL_GRAPHICS_API)
		stringValue = "OpenGL";
	else if (settings.graphicsAPI == OPENGL_ES_GRAPHICS_API)
		stringValue = "OpenGLES";
	else
		abort();

	result &= writeConfString(confWriter,
		"graphicsAPI", stringValue);
	result &= writeConfNewLine(confWriter);

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

inline static GraphicsPipeline createSpritePipelineInstance(
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
		vertexPath = "shaders/vulkan/sprite.vert.spv";
		fragmentPath = "shaders/vulkan/sprite.frag.spv";
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		vertexPath = "shaders/opengl/sprite.vert";
		fragmentPath = "shaders/opengl/sprite.frag";
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

	GraphicsPipeline pipeline;

	MpgxResult mpgxResult = createSpritePipeline(
		framebuffer,
		vertexShader,
		fragmentShader,
		NULL,
		&pipeline);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create sprite pipeline. "
			"(error: %s)", mpgxResultToString(mpgxResult));
		destroyShader(fragmentShader);
		destroyShader(vertexShader);
		return NULL;
	}

	return pipeline;
}
inline static void destroySpritePipelineInstance(
	GraphicsPipeline spritePipeline)
{
	if (!spritePipeline)
		return;

	Shader* shaders = getGraphicsPipelineShaders(spritePipeline);
	Shader vertexShader = shaders[0];
	Shader fragmentShader = shaders[1];
	destroyGraphicsPipeline(spritePipeline);
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
		true,
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

	MpgxResult mpgxResult = createUiWindow32(
		ui,
		title,
		titleLength,
		CENTER_ALIGNMENT_TYPE,
		position,
		scale,
		(cmmt_float_t)24.0,
		(cmmt_float_t)12.0,
		srgbToLinearColor(DEFAULT_UI_BAR_COLOR),
		srgbToLinearColor(DEFAULT_UI_PANEL_COLOR),
		DEFAULT_UI_TEXT_COLOR,
		NULL,
		&events,
		baseWindow,
		true,
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

	mpgxResult = createUiButton32(
		ui,
		text,
		sizeof(text) / sizeof(uint32_t),
		LEFT_ALIGNMENT_TYPE,
		vec3F(
			(cmmt_float_t)12.0,
			(cmmt_float_t)0.0,
			(cmmt_float_t)-0.01),
		valVec2F((cmmt_float_t)16.0),
		(cmmt_float_t)16.0,
		srgbToLinearColor(DEFAULT_UI_DISABLED_BUTTON_COLOR),
		srgbToLinearColor(DEFAULT_UI_ENABLED_BUTTON_COLOR),
		srgbToLinearColor(DEFAULT_UI_HOVERED_BUTTON_COLOR),
		srgbToLinearColor(DEFAULT_UI_PRESSED_BUTTON_COLOR),
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

inline static void destroyMenuWindow(MenuWindow menuWindow)
{
	if (!menuWindow)
		return;

	destroyInterfaceElement(menuWindow->versionLabel);
	destroyBaseWindow(menuWindow->base);
	free(menuWindow);
}
inline static MenuWindow createMenuWindow(
	UserInterface ui,
	Logger logger)
{
	assert(ui);
	assert(logger);

	MenuWindow menuWindow = calloc(
		1, sizeof(MenuWindow_T));

	if (!menuWindow)
		return NULL;

	const uint32_t title[] = {
		'M', 'e', 'n', 'u',
	};

	BaseWindow base = createBaseWindow(
		ui,
		title,
		sizeof(title) / sizeof(uint32_t),
		vec3F(
			(cmmt_float_t)0.0,
			(cmmt_float_t)32.0,
			(cmmt_float_t)0.0),
		vec2F(
			(cmmt_float_t)256.0,
			(cmmt_float_t)64.0),
		logger);

	if (!base)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create UI base window.");
		destroyMenuWindow(menuWindow);
		return NULL;
	}

	menuWindow->base = base;

	Transform windowTransform =
		getInterfaceElementTransform(base->window);

	const char* version = "v" URAN_VERSION_STRING;
	InterfaceElement versionLabel;

	MpgxResult mpgxResult = createUiLabel(
		ui,
		version,
		strlen(version),
		CENTER_ALIGNMENT_TYPE,
		vec3F(
			(cmmt_float_t)0.0,
			(cmmt_float_t)-65.0,
			(cmmt_float_t)-0.001),
		(cmmt_float_t)12.0,
		DEFAULT_UI_TEXT_COLOR,
		true,
		false,
		true,
		true,
		windowTransform,
		NULL,
		NULL,
		true,
		&versionLabel);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create UI label. (error: %s)",
			mpgxResultToString(mpgxResult));
		destroyMenuWindow(menuWindow);
		return NULL;
	}

	menuWindow->versionLabel = versionLabel;
	return menuWindow;
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

	editorInstance->logger = logger;

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

	GraphicsPipeline spritePipeline = createSpritePipelineInstance(
		logger,
		packReader,
		framebuffer);

	if (!spritePipeline)
	{
		destroyEditor(editorInstance);
		destroyPackReader(packReader);
		return NULL;
	}

	editorInstance->spritePipeline = spritePipeline;

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

	float platformScale = calculatePlatformScale(framebuffer);
	int fontSize = (int)(24 * platformScale);

	FontAtlas fontAtlas = createFontAtlasInstance(
		logger,
		packReader,
		textPipeline,
		fontSize);

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
		spritePipeline,
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

	MenuWindow menuWindow = createMenuWindow(
		ui,
		logger);

	if (!menuWindow)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create menu window.");
		destroyEditor(editorInstance);
		destroyPackReader(packReader);
		return NULL;
	}

	editorInstance->menuWindow = menuWindow;

	destroyPackReader(packReader);
	return editorInstance;
}
void destroyEditor(Editor editor)
{
	if (!editor)
		return;

	destroyMenuWindow(editor->menuWindow);
	destroyUserInterface(editor->ui);
	destroyFontAtlasInstance(editor->fontAtlas);
	destroyTextPipelineInstance(editor->textPipeline);
	destroySpritePipelineInstance(editor->spritePipeline);
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

inline static void updateMenuWindow(Editor editor)
{
	assert(editor);

	if (getWindowKeyboardKey(editor->window, U_KEYBOARD_KEY))
	{
		InterfaceElement menuElement = editor->menuWindow->base->window;
		Transform menuTransform = getInterfaceElementTransform(menuElement);
		Vec3F scale = getTransformScale(menuTransform);

		setInterfaceElementPosition(
			menuElement,
			vec3F(
				(cmmt_float_t)0.0,
				scale.y * (cmmt_float_t)0.5,
				(cmmt_float_t)0.0));
		setTransformActive(
			menuTransform,
			true);
	}
}
void updateEditor(Editor editor)
{
	assert(editor);
	updateMenuWindow(editor);
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
