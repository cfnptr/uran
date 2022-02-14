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
#include "uran/editor/base_window.h"

typedef struct MenuWindow_T
{
	void(*onStatsButtonClick)(void*);
	void* handle;
	BaseWindow base;
	InterfaceElement statsButton;
} MenuWindow_T;

typedef MenuWindow_T* MenuWindow;

static void onStatsButtonRelease(InterfaceElement element)
{
	assert(element);
	MenuWindow menuWindow = getUiButtonHandle(element);
	menuWindow->onStatsButtonClick(menuWindow->handle);
}
inline static void destroyMenuWindow(MenuWindow menuWindow)
{
	if (!menuWindow)
		return;

	destroyInterfaceElement(menuWindow->statsButton);
	destroyBaseWindow(menuWindow->base);
	free(menuWindow);
}
inline static MenuWindow createMenuWindow(
	UserInterface ui,
	Logger logger,
	void(*onStatsButtonClick)(void*),
	void* handle)
{
	assert(ui);
	assert(logger);
	assert(onStatsButtonClick);
	assert(handle);

	MenuWindow menuWindow = calloc(
		1, sizeof(MenuWindow_T));

	if (!menuWindow)
		return NULL;

	menuWindow->onStatsButtonClick = onStatsButtonClick;
	menuWindow->handle = handle;

	BaseWindow base = createBaseWindow(ui,
		"Menu",
		vec2F(
			(cmmt_float_t)256.0,
			(cmmt_float_t)128.0),
		logger);

	if (!base)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create base window.");
		destroyMenuWindow(menuWindow);
		return NULL;
	}

	menuWindow->base = base;

	Transform windowTransform =
		getInterfaceElementTransform(base->window);

	InterfaceElementEvents events = emptyInterfaceElementEvents;
	events.onRelease = onStatsButtonRelease;
	const char* statsLabel = "Stats";

	InterfaceElement statsButton;

	MpgxResult mpgxResult = createUiButton8(ui,
		statsLabel,
		strlen(statsLabel),
		TOP_ALIGNMENT_TYPE,
		vec3F(
			(cmmt_float_t)0.0,
			(cmmt_float_t)-60.0,
			(cmmt_float_t)-0.01),
		vec2F(
			(cmmt_float_t)224.0,
			(cmmt_float_t)32.0),
		true,
		windowTransform,
		&events,
		menuWindow,
		true,
		&statsButton);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create UI button. (error: %s)",
			mpgxResultToString(mpgxResult));
		destroyMenuWindow(menuWindow);
		return NULL;
	}

	menuWindow->statsButton = statsButton;
	return menuWindow;
}
