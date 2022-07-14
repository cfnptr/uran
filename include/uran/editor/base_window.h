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
#include "uran/user_interface.h"
#include "cmmt/angle.h"

typedef struct BaseWindow_T
{
	InterfaceElement window;
	InterfaceElement closeButton;
} BaseWindow_T;

typedef BaseWindow_T* BaseWindow;

static void onBaseWindowCloseRelease(InterfaceElement element)
{
	assert(element);
	BaseWindow baseWindow = getUiButtonHandle(element);
	Transform transform = getInterfaceElementTransform(baseWindow->window);
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
	const char* title,
	Vec2F scale,
	Logger logger)
{
	assert(ui);
	assert(title);
	assert(scale.x > 0.0f);
	assert(scale.y > 0.0f);
	assert(logger);

	BaseWindow baseWindow = calloc(
		1, sizeof(BaseWindow_T));

	if (!baseWindow)
		return NULL;

	InterfaceElement window;

	MpgxResult mpgxResult = createUiWindow8(ui,
		title,
		strlen(title),
		CENTER_ALIGNMENT_TYPE,
		zeroVec3F,
		scale,
		NULL,
		NULL,
		NULL,
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

	InterfaceElementEvents events = emptyInterfaceElementEvents;
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
	setTransformRotationType(
		crossTransform,
		SPIN_ROTATION_TYPE);
	setTransformEulerAngles(
		crossTransform,
		vec3F(
			(cmmt_float_t)0.0,
			(cmmt_float_t)0.0,
			degToRad((cmmt_float_t)45.0)));

	baseWindow->closeButton = closeButton;
	return baseWindow;
}
