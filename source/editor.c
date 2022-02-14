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
#include "uran/editor/menu_window.h"
#include "uran/editor/stats_window.h"

#include "mpmt/common.h"

struct Editor_T
{
	Logger logger;
	Window window;
	MenuWindow menuWindow;
	StatsWindow statsWindow;
};

static void onStatsButtonClick(void* handle)
{
	assert(handle);
	Editor editor = (Editor)handle;

	InterfaceElement window = editor->statsWindow->base->window;
	setInterfaceElementPosition(window, vec3F(
		(cmmt_float_t)448.0, (cmmt_float_t)-160.0, (cmmt_float_t)0.0));
	setTransformActive(getInterfaceElementTransform(
		window), true);
}
Editor createEditor(
	Logger logger,
	Window window,
	UserInterface ui)
{
	assert(logger);
	assert(window);
	assert(ui);

	Editor editor = calloc(
		1, sizeof(Editor_T));

	if (!editor)
		return NULL;

	editor->logger = logger;
	editor->window = window;

	MenuWindow menuWindow = createMenuWindow(
		ui, logger,
		onStatsButtonClick,
		editor);

	if (!menuWindow)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create menu window.");
		destroyEditor(editor);
		return NULL;
	}

	editor->menuWindow = menuWindow;

	StatsWindow statsWindow = createStatsWindow(ui, logger, window);

	if (!statsWindow)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create stats window.");
		destroyEditor(editor);
		return NULL;
	}

	editor->statsWindow = statsWindow;
	return editor;
}
void destroyEditor(Editor editor)
{
	if (!editor)
		return;

	destroyStatsWindow(editor->statsWindow);
	destroyMenuWindow(editor->menuWindow);
	free(editor);
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

	if (getWindowKeyboardKey(editor->window, M_KEYBOARD_KEY))
	{
		InterfaceElement window = editor->menuWindow->base->window;
		setInterfaceElementPosition(window, zeroVec3F);
		Transform transform = getInterfaceElementTransform(window);
		setTransformActive(transform, true);
	}
}
void postUpdateEditor(Editor editor)
{
	assert(editor);

	Transform statsTransform = getInterfaceElementTransform(
		editor->statsWindow->base->window);

	if (isTransformActive(statsTransform))
	{
		editor->statsWindow->cpuTime = (float)((getCurrentClock() -
			getWindowUpdateTime(editor->window)) * 1000.0);
	}
}
