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
#include "uran/interface.h"
#include "uran/renderers/text_renderer.h"
#include "uran/renderers/sprite_renderer.h"

#define DEFAULT_UI_PANEL_COLOR srgbColor(48, 48, 48, 255)
#define DEFAULT_UI_BAR_COLOR srgbColor(64, 64, 64, 255)
#define DEFAULT_UI_TEXT_COLOR srgbColor(240, 240, 240, 255)
#define DEFAULT_UI_ENABLED_BUTTON_COLOR srgbColor(80, 80, 80, 255)
#define DEFAULT_UI_DISABLED_BUTTON_COLOR srgbColor(64, 64, 64, 255)
#define DEFAULT_UI_HOVERED_BUTTON_COLOR srgbColor(96, 96, 96, 255)
#define DEFAULT_UI_PRESSED_BUTTON_COLOR srgbColor(64, 64, 64, 255)

/*
 * User interface structure.
 */
typedef struct UserInterface_T UserInterface_T;
/*
 * User interface instance.
 */
typedef UserInterface_T* UserInterface;

/*
 * Create a new user interface instance.
 * Returns operation MPGX result.
 *
 * transformer - transformer instance.
 * spritePipeline - sprite pipeline instance.
 * textPipeline - text pipeline instance.
 * fontAtlas - font atlas instance.
 * scale - interface scale.
 * capacity - initial element array capacity or 0.
 * threadPool - thread pool instance or NULL.
 * ui - pointer to the UI instance.
 */
MpgxResult createUserInterface(
	Transformer transformer,
	GraphicsPipeline spritePipeline,
	GraphicsPipeline textPipeline,
	FontAtlas fontAtlas,
	cmmt_float_t scale,
	size_t capacity,
	ThreadPool threadPool,
	UserInterface* ui);
/*
 * Destroys user interface instance.
 * ui - user interface instance or NULL.
 */
void destroyUserInterface(UserInterface ui);

/*
 * Returns user interface transformer.
 * ui - user interface instance.
 */
Transformer getUserInterfaceTransformer(UserInterface ui);
/*
 * Returns user interface sprite pipeline.
 * ui - user interface instance.
 */
GraphicsPipeline getUserInterfaceSpritePipeline(UserInterface ui);
/*
 * Returns user interface text pipeline.
 * ui - user interface instance.
 */
GraphicsPipeline getUserInterfaceTextPipeline(UserInterface ui);
/*
 * Returns user interface font atlas.
 * ui - user interface instance.
 */
FontAtlas getUserInterfaceFontAtlas(UserInterface ui);
/*
 * Returns user interface instance.
 * ui - user interface instance.
 */
Interface getUserInterface(UserInterface ui);

/*
 * Processes user interface events.
 * ui - user interface instance.
 */
void updateUserInterface(UserInterface ui);
/*
 * Draw user interface elements.
 * ui - user interface instance.
 */
GraphicsRendererResult drawUserInterface(UserInterface ui);

/*
 * Create a new UI panel instance.
 * Returns operation MPGX result.
 *
 * ui - user interface instance.
 * alignment - alignment type.
 * position - panel position.
 * scale - panel scale.
 * color - panel color.
 * parent - parent instance or NULL.
 * events - interface events or NULL.
 * handle - panel handle or NULL.
 * isActive - is panel active.
 * uiPanel - pointer to the UI panel.
 */
MpgxResult createUiPanel(
	UserInterface ui,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	LinearColor color,
	Transform parent,
	const InterfaceElementEvents* events,
	void* handle,
	bool isActive,
	InterfaceElement* uiPanel);
/*
 * Returns UI panel handle.
 * panel - UI panel instance.
 */
void* getUiPanelHandle(InterfaceElement panel);
/*
 * Returns UI panel sprite render instance.
 * panel - UI panel instance.
 */
GraphicsRender getUiPanelRender(InterfaceElement panel);

/*
 * Create a new UTF-32 UI label instance.
 * Returns operation MPGX result.
 *
 * ui - user interface instance.
 * string - label string.
 * stringLength - label string length.
 * alignment - alignment type.
 * position - label position.
 * scale - label scale.
 * color - label color.
 * useTags - use HTML tags.
 * isConstant - is label constant
 * parent - parent instance or NUL.
 * events - interface events or NULL.
 * handle - label handle or NULL.
 * isActive - is label active.
 * uiPanel - pointer to the UI label.
 */
MpgxResult createUiLabel32(
	UserInterface ui,
	const uint32_t* string,
	size_t stringLength,
	AlignmentType alignment,
	Vec3F position,
	cmmt_float_t scale,
	SrgbColor color,
	bool useTags,
	bool isBold,
	bool isItalic,
	bool isConstant,
	Transform parent,
	const InterfaceElementEvents* events,
	void* handle,
	bool isActive,
	InterfaceElement* uiLabel);
/*
 * Create a new UTF-8 UI label instance.
 * Returns operation MPGX result.
 *
 * ui - user interface instance.
 * string - label string.
 * stringLength - label string length.
 * alignment - alignment type.
 * position - label position.
 * scale - label scale.
 * color - label color.
 * useTags - use HTML tags.
 * isConstant - is label constant
 * parent - parent instance or NULL.
 * events - interface events or NULL.
 * handle - label handle or NULL.
 * isActive - is label active.
 * uiPanel - pointer to the UI label.
 */
MpgxResult createUiLabel(
	UserInterface ui,
	const char* string,
	size_t stringLength,
	AlignmentType alignment,
	Vec3F position,
	cmmt_float_t scale,
	SrgbColor color,
	bool useTags,
	bool isBold,
	bool isItalic,
	bool isConstant,
	Transform parent,
	const InterfaceElementEvents* events,
	void* handle,
	bool isActive,
	InterfaceElement* uiLabel);
/*
 * Returns UI label handle.
 * label - UI label instance.
 */
void* getUiLabelHandle(InterfaceElement label);
/*
 * Returns UI label text render instance.
 * label - UI label instance.
 */
GraphicsRender getUiLabelRender(InterfaceElement label);

/*
 * Create a new UTF-32 UI window instance.
 * Returns operation MPGX result.
 *
 * ui - user interface instance.
 * title - title string.
 * titleLength - title string length.
 * alignment - alignment type.
 * position - window position.
 * scale - window scale.
 * barHeight - window bar height.
 * titleHeight - window title height.
 * barColor - window bar color.
 * panelColor - window panel color.
 * titleColor - window title color.
 * parent - parent instance or NULL.
 * isActive - is window active.
 * events - interface events or NULL.
 * handle - window handle or NULL.
 * uiPanel - pointer to the UI window.
 */
MpgxResult createUiWindow32(
	UserInterface ui,
	const uint32_t* title,
	size_t titleLength,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	cmmt_float_t barHeight,
	cmmt_float_t titleHeight,
	LinearColor barColor,
	LinearColor panelColor,
	SrgbColor titleColor,
	Transform parent,
	const InterfaceElementEvents* events,
	void* handle,
	bool isActive,
	InterfaceElement* uiWindow);
/*
 * Create a new UTF-8 UI window instance.
 * Returns operation MPGX result.
 *
 * ui - user interface instance.
 * title - title string.
 * titleLength - title string length.
 * alignment - alignment type.
 * position - window position.
 * scale - window scale.
 * barHeight - window bar height.
 * titleHeight - window title height.
 * barColor - window bar color.
 * panelColor - window panel color.
 * titleColor - window title color.
 * parent - parent instance or NULL.
 * events - interface events or NULL.
 * handle - window handle or NULL.
 * isActive - is window active.
 * uiPanel - pointer to the UI window.
 */
MpgxResult createUiWindow(
	UserInterface ui,
	const char* title,
	size_t titleLength,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	cmmt_float_t barHeight,
	cmmt_float_t titleHeight,
	LinearColor barColor,
	LinearColor panelColor,
	SrgbColor titleColor,
	Transform parent,
	const InterfaceElementEvents* events,
	void* handle,
	bool isActive,
	InterfaceElement* uiWindow);
/*
 * Returns UI window handle.
 * window - UI window instance.
 */
void* getUiWindowHandle(InterfaceElement window);
/*
 * Returns UI window panel sprite render instance.
 * window - UI window instance.
 */
GraphicsRender getUiWindowPanelRender(InterfaceElement window);
/*
 * Returns UI window bar sprite render instance.
 * window - UI window instance.
 */
GraphicsRender getUiWindowBarRender(InterfaceElement window);
/*
 * Returns UI window title text render instance.
 * window - UI window instance.
 */
GraphicsRender getUiWindowTitleRender(InterfaceElement window);

// TODO: window event getters

MpgxResult createUiButton32(
	UserInterface ui,
	const uint32_t* text,
	size_t textLength,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	cmmt_float_t textHeight,
	LinearColor disabledColor,
	LinearColor enabledColor,
	LinearColor hoveredColor,
	LinearColor pressedColor,
	SrgbColor textColor,
	Transform parent,
	const InterfaceElementEvents* events,
	void* handle,
	bool isEnabled,
	bool isActive,
	InterfaceElement* uiButton);
MpgxResult createUiButton(
	UserInterface ui,
	const char* text,
	size_t textLength,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	cmmt_float_t textHeight,
	LinearColor disabledColor,
	LinearColor enabledColor,
	LinearColor hoveredColor,
	LinearColor pressedColor,
	SrgbColor textColor,
	Transform parent,
	const InterfaceElementEvents* events,
	void* handle,
	bool isEnabled,
	bool isActive,
	InterfaceElement* uiButton);
/*
 * Returns UI button handle.
 * button - UI button instance.
 */
void* getUiButtonHandle(InterfaceElement button);
/*
 * Returns UI button panel sprite render instance.
 * button - UI button instance.
 */
GraphicsRender getUiButtonPanelRender(InterfaceElement button);
/*
 * Returns UI button text render instance.
 * button - UI button instance.
 */
GraphicsRender getUiButtonTextRender(InterfaceElement button);
