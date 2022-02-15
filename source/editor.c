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
#include "conf/reader.h"
#include "conf/writer.h"

#include <string.h>

#define SETTINGS_FILE_PATH "editor-settings.txt"
#define ENGINE_NAME "Uran"
#define APPLICATION_NAME "Editor"
#define WINDOW_TITLE "Uran Editor"

typedef struct Settings
{
	GraphicsAPI graphicsAPI;
} Settings;
struct Editor_T
{
	Logger logger;
	Transformer transformer;
	Window window;
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
			"Failed to crete settings reader. (error: %s at line %zu)",
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

Editor createEditor(
	Logger logger,
	Transformer transformer,
	OnWindowUpdate onUpdate,
	void* updateArgument)
{
	assert(logger);
	assert(transformer);
	assert(onUpdate);
	assert(updateArgument);

	Editor editorInstance = calloc(1,
		sizeof(Editor_T));

	if (!editorInstance)
		return NULL;

	editorInstance->logger = logger;
	editorInstance->transformer = transformer;

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
		return NULL;
	}

	Window window;

	mpgxResult = createWindow(
		onUpdate,
		updateArgument,
		false,
		false,
		false,
		NULL,
		&window);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create window. (error: %s)",
			mpgxResultToString(mpgxResult));
		terminateGraphics();
		return NULL;
	}

	editorInstance->window = window;

	setWindowTitle(window, WINDOW_TITLE);
	return editorInstance;
}
void destroyEditor(Editor editor)
{
	if (!editor)
		return;

	destroyWindow(editor->window);
	terminateGraphics();
	storeSettings(
		editor->logger,
		editor->settings);
	free(editor);
}

Window getEditorWindow(Editor editor)
{
	assert(editor);
	return editor->window;
}

void updateEditor(Editor editor)
{
	assert(editor);
}
void renderEditor(Editor editor)
{
	assert(editor);

	Framebuffer framebuffer = getWindowFramebuffer(
		editor->window);

	beginFramebufferRender(
		framebuffer,
		NULL,
		0);

	// TODO

	endFramebufferRender(framebuffer);
}
