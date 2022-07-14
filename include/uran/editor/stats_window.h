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
#include <stdio.h>

#define STATS_WINDOW_UPDATE_DELAY 0.05

typedef struct StatsWindow_T
{
	Logger logger;
	Window window;
	BaseWindow base;
	InterfaceElement label;
	double lastUpdateTime;
	double deltaTime;
	GraphicsRendererResult rendererResult;
	float cpuTime;
} StatsWindow_T;

typedef StatsWindow_T* StatsWindow;

static void onStatsLabelUpdate(InterfaceElement element)
{
	assert(element);
	StatsWindow statsWindow = getUiLabelHandle(element);
	Window window = statsWindow->window;
	double updateTime = getWindowUpdateTime(window);

	if (updateTime < statsWindow->lastUpdateTime)
		return;

	double deltaTime = (statsWindow->deltaTime + getWindowDeltaTime(window)) * 0.5;
	statsWindow->deltaTime = deltaTime;
	GraphicsRendererResult rendererResult = statsWindow->rendererResult;
	Framebuffer framebuffer = getWindowFramebuffer(window);
	Vec2I framebufferSize = getFramebufferSize(framebuffer);

	char buffer[512];

	// TODO: GPU time

	size_t count = snprintf(
		buffer, 512,
		"FPS: <b>%d (<i>%dms</i>)</b>\n"
		"CPU time: <b>%.3fms</b>\n"
		"Draw count: <b>%zu</b>\n"
		"Polygon count: <b>%zu</b>\n"
		"Pass count: <b>%zu</b>\n"
		"Buffer count: <b>%zu</b>\n"
		"Image count: <b>%zu</b>\n"
		"Sampler count: <b>%zu</b>\n"
		"Framebuffer count: <b>%zu</b>\n"
		"Shader count: <b>%zu</b>\n"
		"Graphics mesh count: <b>%zu</b>\n"
		"Compute pipeline count: <b>%zu</b>\n"
		"Main FB pipeline count: <b>%zu</b>\n"
		"Main FB size: <b>%dx%d</b>\n",
		(int)(1.0 / deltaTime),
		(int)(deltaTime * 1000.0),
		(float)(statsWindow->cpuTime),
		rendererResult.drawCount,
		rendererResult.indexCount / 3,
		rendererResult.passCount,
		getWindowBufferCount(window),
		getWindowImageCount(window),
		getWindowSamplerCount(window),
		getWindowFramebufferCount(window) + 1,
		getWindowShaderCount(window),
		getWindowGraphicsMeshCount(window),
		getWindowComputePipelineCount(window),
		getFramebufferPipelineCount(framebuffer),
		framebufferSize.x, framebufferSize.y);

	MpgxResult mpgxResult = setUiLabelText8(
		element, buffer, count);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(statsWindow->logger, FATAL_LOG_LEVEL,
			"Failed to set stats label text. (error: %s)",
			mpgxResultToString(mpgxResult));
		abort();
	}

	Text text = getTextRenderText(getUiLabelRender(element));
	Vec2F textSize = getTextSize(text);
	AlignmentType alignment = getTextAlignment(text);

	setInterfaceElementBounds(element,
		createTextBox2F(alignment, textSize));

	statsWindow->lastUpdateTime = updateTime + STATS_WINDOW_UPDATE_DELAY;
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

	statsWindow->logger = logger;
	statsWindow->window = window;
	statsWindow->lastUpdateTime = 0.0;
	statsWindow->deltaTime = 0.0;
	statsWindow->rendererResult = createGraphicsRendererResult();
	statsWindow->cpuTime = 0.0f;

	BaseWindow base = createBaseWindow(ui,
		"Stats",
		vec2F(
			(cmmt_float_t)256.0,
			(cmmt_float_t)256.0),
		logger);

	if (!base)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create base window.");
		destroyStatsWindow(statsWindow);
		return NULL;
	}

	statsWindow->base = base;

	Transform windowTransform =
		getInterfaceElementTransform(base->window);

	InterfaceElementEvents events = emptyInterfaceElementEvents;
	events.onUpdate = onStatsLabelUpdate;

	InterfaceElement label;

	MpgxResult mpgxResult = createUiLabel(ui,
		NULL,
		0,
		LEFT_TOP_ALIGNMENT_TYPE,
		vec3F(
			(cmmt_float_t)16.0,
			(cmmt_float_t)-44.0,
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
