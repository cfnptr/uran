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

#include <string.h>

#define SETTINGS_FILE_PATH "editor-settings.txt"
#define ENGINE_NAME "Uran"
#define APPLICATION_NAME "Editor"
#define WINDOW_TITLE "Uran Editor"

#define EDITOR_WINDOW_PANEL_COLOR srgbToLinearColor(srgbColor(48, 48, 48, 255))
#define EDITOR_WINDOW_BAR_COLOR srgbToLinearColor(srgbColor(64, 64, 64, 255))
#define EDITOR_ENABLED_BUTTON_COLOR srgbToLinearColor(srgbColor(80, 80, 80, 255))
#define EDITOR_DISABLED_BUTTON_COLOR srgbToLinearColor(srgbColor(64, 64, 64, 255))
#define EDITOR_HOVERED_BUTTON_COLOR srgbToLinearColor(srgbColor(96, 96, 96, 255))
#define EDITOR_PRESSED_BUTTON_COLOR srgbToLinearColor(srgbColor(64, 64, 64, 255))

/*#define INPUT_PANEL_COLOR srgbToLinearColor(srgbColor(32, 32, 32, 255))
#define FOCUSED_INPUT_COLOR srgbToLinearColor(srgbColor(128, 128, 128, 255))
#define INCORRECT_INPUT_COLOR srgbToLinearColor(srgbColor(192, 32, 32, 255))
#define DEFAULT_TEXT_COLOR srgbToLinearColor(srgbColor(240, 240, 240, 255))
#define PLACEHOLDER_TEXT_COLOR srgbToLinearColor(srgbColor(144, 144, 144, 255))
#define CHECKBOX_CHECK_COLOR srgbToLinearColor(srgbColor(128, 128, 128, 255))*/

typedef struct MenuWindow_T
{
	InterfaceElement window;
	InterfaceElement closeButton;
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
		SETTINGS_FILE_PATH,
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

	GraphicsPipeline spritePipeline;

	MpgxResult mpgxResult = createSpritePipeline(
		framebuffer,
		vertexShader,
		fragmentShader,
		NULL,
		&spritePipeline);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create sprite pipeline. "
			"(error: %s)", mpgxResultToString(mpgxResult));
		destroyShader(fragmentShader);
		destroyShader(vertexShader);
		return NULL;
	}

	return spritePipeline;
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

inline static void destroyMenuWindow(
	MenuWindow menuWindow)
{
	if (!menuWindow)
		return;

	destroyInterfaceElement(menuWindow->closeButton);
	destroyInterfaceElement(menuWindow->window);
	free(menuWindow);
}
inline static MenuWindow createMenuWindow(
	UserInterface ui)
{
	assert(ui);

	MenuWindow menuWindow = calloc(
		1, sizeof(MenuWindow_T));

	if (!menuWindow)
		return NULL;

	InterfaceElement window = createUiWindow(ui,
		CENTER_ALIGNMENT_TYPE,
		zeroVec3F,
		vec2F(
			(cmmt_float_t)256.0,
			(cmmt_float_t)64.0),
		(cmmt_float_t)24.0,
		EDITOR_WINDOW_BAR_COLOR,
		EDITOR_WINDOW_PANEL_COLOR,
		NULL,
		true);

	if (!window)
	{
		destroyMenuWindow(menuWindow);
		return NULL;
	}

	menuWindow->window = window;

	InterfaceElement closeButton = createUiButton(ui,
		LEFT_ALIGNMENT_TYPE, // TODO: make inherited left alignment
		vec3F(
			(cmmt_float_t)12.0,
			(cmmt_float_t)0.0,
			(cmmt_float_t)-0.001),
		valVec2F((cmmt_float_t)16.0),
		EDITOR_DISABLED_BUTTON_COLOR,
		EDITOR_ENABLED_BUTTON_COLOR,
		EDITOR_HOVERED_BUTTON_COLOR,
		EDITOR_PRESSED_BUTTON_COLOR,
		getInterfaceElementTransform(window),
		true,
		true);

	if (!closeButton)
	{
		destroyMenuWindow(menuWindow);
		return NULL;
	}

	menuWindow->closeButton = closeButton;
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
		"resources.pack",
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
			"Failed to initialize graphics. (error: %s)",
			mpgxResultToString(mpgxResult));
		destroyEditor(editorInstance);
		destroyPackReader(packReader);
		return NULL;
	}

	Transformer transformer = createTransformer(
		MPGX_DEFAULT_CAPACITY, threadPool);

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

	UserInterface ui;

	mpgxResult = createUserInterface(
		transformer,
		spritePipeline,
		spritePipeline, // TODO text pipeline
		MPGX_DEFAULT_CAPACITY,
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

	MenuWindow menuWindow = createMenuWindow(ui);

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
	// TODO: destroy text pipeline
	destroySpritePipelineInstance(editor->spritePipeline);
	destroyWindow(editor->window);
	destroyTransformer(editor->transformer);
	terminateGraphics();
	storeSettings(
		editor->logger,
		editor->settings);
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
