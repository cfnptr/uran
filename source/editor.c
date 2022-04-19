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
#include "cmmt/angle.h"
#include "mpmt/common.h"

#include <stdio.h>

#if _WIN32
#undef interface
#endif

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
	float cpuTime;
} StatsWindow_T;

typedef StatsWindow_T* StatsWindow;

typedef struct MenuBar_T
{
	Window window;
	Interface interface;
	StatsWindow statsWindow;
	InterfaceElement panel;
	InterfaceElement statsButton;
} MenuBar_T;

typedef MenuBar_T* MenuBar;

struct Editor_T
{
	Logger logger;
	StatsWindow statsWindow;
	MenuBar menuBar;
};

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

	MpgxResult mpgxResult = createUiWindow(ui,
		title,
		titleLength,
		CENTER_ALIGNMENT_TYPE,
		position,
		scale,
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

	const uint32_t text[] = {
		'<', '/', 'b', '>', '+'
	};

	events.onEnter = onBaseWindowCloseEnter;
	events.onExit = onBaseWindowCloseExit;
	events.onRelease = onBaseWindowCloseRelease;

	InterfaceElement closeButton;

	mpgxResult = createUiButton(ui,
		text,
		sizeof(text) / sizeof(uint32_t),
		LEFT_ALIGNMENT_TYPE,
		vec3F(
			(cmmt_float_t)14.0,
			scale.y * (cmmt_float_t)0.5,
			(cmmt_float_t)-0.01),
		valVec2F((cmmt_float_t)18.0),
		true,
		windowTransform,
		&events,
		baseWindow,
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
			(cmmt_float_t)18.0,
			(cmmt_float_t)18.0,
			(cmmt_float_t)1.0));
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

	char buffer[256];

	// TODO: GPU time

	size_t count = snprintf(
		buffer,
		256,
		"<b>FPS</b>: %d (<i>%dms</i>)\n"
		"<b>CPU time</b>: %.3fms\n"
		"<b>Draw count</b>: %zu\n"
		"<b>Polygon count</b>: %zu\n"
		"<b>Pass count</b>: %zu",
		(int)(1.0 / deltaTime),
		(int)(deltaTime * 1000.0),
		(float)(statsWindow->cpuTime),
		rendererResult.drawCount,
		rendererResult.indexCount / 3,
		rendererResult.passCount);

	bool result = setTextString8(text,
		buffer, count);

	if (!result)
	{
		logMessage(statsWindow->logger, FATAL_LOG_LEVEL,
			"Failed to set stats text.");
		abort();
	}

	MpgxResult mpgxResult = bakeText(text);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(statsWindow->logger, FATAL_LOG_LEVEL,
			"Failed to bake stats text. (error: %s)",
			mpgxResultToString(mpgxResult));
		abort();
	}

	Vec2F textSize = getTextSize(text);
	AlignmentType alignment = getTextAlignment(text);

	setInterfaceElementBounds(element,
		createTextBox2F(alignment, textSize));

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

	statsWindow->lastUpdateTime = 0.0;
	statsWindow->rendererResult = createGraphicsRendererResult();
	statsWindow->cpuTime = 0.0f;

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
		'L', 'o', 'a', 'd',
		'i', 'n', 'g', '.', '.', '.',
	};

	InterfaceElementEvents events = emptyInterfaceElementEvents;
	events.onUpdate = onStatsLabelUpdate;

	InterfaceElement label;

	MpgxResult mpgxResult = createUiLabel(ui,
		labelText,
		sizeof(labelText) / sizeof(uint32_t),
		LEFT_TOP_ALIGNMENT_TYPE,
		vec3F(
			(cmmt_float_t)16.0,
			(cmmt_float_t)-40.0,
			(cmmt_float_t)-0.001),
		(cmmt_float_t)DEFAULT_UI_TEXT_HEIGHT,
		DEFAULT_UI_TEXT_COLOR,
		false,
		false,
		true,
		false,
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
		(cmmt_float_t)windowSize.x /
			getInterfaceScale(menuBar->interface),
		(cmmt_float_t)(DEFAULT_UI_TEXT_HEIGHT * 2.0),
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
	menuBar->interface = getUserInterface(ui);
	menuBar->statsWindow = statsWindow;

	InterfaceElementEvents events = emptyInterfaceElementEvents;
	events.onUpdate = onMenuBarUpdate;

	InterfaceElement element;

	MpgxResult mpgxResult = createUiPanel(ui,
		TOP_ALIGNMENT_TYPE,
		vec3F(
			(cmmt_float_t)0.0,
			(cmmt_float_t)-14.0,
			(cmmt_float_t)-0.1),
		vec2F(
			(cmmt_float_t)defaultWindowSize.x /
				getInterfaceScale(menuBar->interface),
			(cmmt_float_t)(DEFAULT_UI_TEXT_HEIGHT * 2.0)),
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

	mpgxResult = createUiButton(ui,
		statsText,
		sizeof(statsText) / sizeof(uint32_t),
		LEFT_ALIGNMENT_TYPE,
		vec3F(
			(cmmt_float_t)32.0,
			(cmmt_float_t)0.0,
			(cmmt_float_t)-0.01),
		vec2F(
			(cmmt_float_t)64.0,
			(cmmt_float_t)(DEFAULT_UI_TEXT_HEIGHT * 2.0)),
		true,
		panelTransform,
		&events,
		menuBar,
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

	StatsWindow statsWindow = createStatsWindow(
		ui,
		logger,
		window);

	if (!statsWindow)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create stats window.");
		destroyEditor(editor);
		return NULL;
	}

	editor->statsWindow = statsWindow;

	MenuBar menuBar = createMenuBar(
		ui,
		logger,
		window,
		statsWindow);

	if (!menuBar)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create menu bar.");
		destroyEditor(editor);
		return NULL;
	}

	editor->menuBar = menuBar;
	return editor;
}
void destroyEditor(Editor editor)
{
	if (!editor)
		return;

	destroyMenuBar(editor->menuBar);
	destroyStatsWindow(editor->statsWindow);
	free(editor);
}

void setEditorRendererResult(
	Editor editor,
	GraphicsRendererResult result)
{
	assert(editor);
	editor->statsWindow->rendererResult = result;
}

void postUpdateEditor(Editor editor)
{
	assert(editor);

	Transform statsTransform = getInterfaceElementTransform(
		editor->statsWindow->base->window);

	if (isTransformActive(statsTransform))
	{
		editor->statsWindow->cpuTime = (float)((getCurrentClock() -
			getWindowUpdateTime(editor->statsWindow->window)) * 1000.0);
	}
}
