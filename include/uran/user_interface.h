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
#include "uran/renderers/panel_renderer.h"

#define DEFAULT_UI_BAR_HEIGHT 28
#define DEFAULT_UI_TEXT_HEIGHT 14

#define DEFAULT_UI_PANEL_COLOR srgbColor(48, 48, 48, 255)
#define DEFAULT_UI_BAR_COLOR srgbColor(80, 80, 80, 255)
#define DEFAULT_UI_TEXT_COLOR srgbColor(248, 248, 248, 255)
#define DEFAULT_UI_ENABLED_BUTTON_COLOR srgbColor(80, 80, 80, 255)
#define DEFAULT_UI_DISABLED_BUTTON_COLOR srgbColor(64, 64, 64, 255)
#define DEFAULT_UI_HOVERED_BUTTON_COLOR srgbColor(96, 96, 96, 255)
#define DEFAULT_UI_PRESSED_BUTTON_COLOR srgbColor(64, 64, 64, 255)
#define DEFAULT_UI_INPUT_PANEL_COLOR srgbColor(32, 32, 32, 255)
#define DEFAULT_UI_ENABLED_INPUT_COLOR srgbColor(80, 80, 80, 255)
#define DEFAULT_UI_DISABLED_INPUT_COLOR srgbColor(64, 64, 64, 255)
#define DEFAULT_UI_FOCUSED_INPUT_COLOR srgbColor(128, 128, 128, 255)
#define DEFAULT_UI_PLACEHOLDER_COLOR srgbColor(144, 144, 144, 255)
#define DEFAULT_UI_ENABLED_CHECKBOX_COLOR srgbColor(32, 32, 32, 255)
#define DEFAULT_UI_DISABLED_CHECKBOX_COLOR srgbColor(16, 16, 16, 255)
#define DEFAULT_UI_HOVERED_CHECKBOX_COLOR srgbColor(48, 48, 48, 255)
#define DEFAULT_UI_PRESSED_CHECKBOX_COLOR srgbColor(24, 24, 24, 255)
#define DEFAULT_UI_CHECKBOX_FOCUS_COLOR srgbColor(80, 80, 80, 255)
#define DEFAULT_UI_CHECKBOX_CHECK_COLOR srgbColor(128, 128, 128, 255)

/*
 * Use interface element types.
 */
typedef enum UiType_T
{
	PANEL_UI_TYPE = 0,
	LABEL_UI_TYPE = 1,
	WINDOW_UI_TYPE = 2,
	BUTTON_UI_TYPE = 3,
	INPUT_FIELD_UI_TYPE = 4,
	CHECKBOX_UI_TYPE = 5,
	CUSTOM_UI_TYPES = 6,
	// Note: your custom UI types...
} UiType_T;
/*
 * Use interface element type.
 */
typedef size_t UiType;

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
 * panelPipeline - panel pipeline instance.
 * textPipeline - text pipeline instance.
 * fontAtlases - font atlas array.
 * fontAtlasCount - font atlas array size.
 * scale - interface scale.
 * capacity - initial element array capacity.
 * threadPool - thread pool instance or NULL.
 * ui - pointer to the UI instance.
 */
MpgxResult createUserInterface(
	GraphicsPipeline panelPipeline,
	GraphicsPipeline textPipeline,
	FontAtlas* fontAtlases,
	size_t fontAtlasCount,
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
 * Returns user interface panel renderer.
 * ui - user interface instance.
 */
GraphicsRenderer getUserInterfacePanelRenderer(UserInterface ui);
/*
 * Returns user interface text renderer.
 * ui - user interface instance.
 */
GraphicsRenderer getUserInterfaceTextRenderer(UserInterface ui);
/*
 * Returns user interface font atlas array.
 * ui - user interface instance.
 */
FontAtlas* getUserInterfaceFontAtlases(UserInterface ui);
/*
 * Returns user interface font atlas array size.
 * ui - user interface instance.
 */
size_t getUserInterfaceFontAtlasCount(UserInterface ui);
/*
 * Returns user interface transformer.
 * ui - user interface instance.
 */
Transformer getUserInterfaceTransformer(UserInterface ui);
/*
 * Returns user interface instance.
 * ui - user interface instance.
 */
Interface getUserInterface(UserInterface ui);
/*
 * Returns user interface cursor render.
 * ui - user interface instance.
 */
GraphicsRender getUserInterfaceCursor(UserInterface ui);

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
 * Defocus focussed interface element.
 * ui - user interface instance.
 */
void defocusUserInterface(UserInterface ui);

/*
 * Returns user interface element type.
 * element - user interface element instance.
 */
UiType getUiType(InterfaceElement element);

/*
 * Create a new UI panel instance.
 * Returns operation MPGX result.
 *
 * ui - user interface instance.
 * alignment - alignment type.
 * position - panel position.
 * scale - panel scale.
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
 * Returns UI panel render instance.
 * panel - UI panel instance.
 */
GraphicsRender getUiPanelRender(InterfaceElement panel);

/*
 * Create a new UTF-32 UI label instance.
 * Returns operation MPGX result.
 *
 * ui - user interface instance.
 * string - label string or NULL.
 * stringLength - label string length or 0.
 * alignment - alignment type.
 * position - label position.
 * scale - label scale.
 * color - initial label color.
 * isBold - is label bold initially.
 * isItalic - is label italic initially.
 * useTags - use HTML tags.
 * isConstant - is label constant.
 * isUniversal - support any UTF character. (Heavy)
 * parent - parent instance or NUL.
 * events - interface events or NULL.
 * handle - label handle or NULL.
 * isActive - is label active.
 * uiPanel - pointer to the UI label.
 */
MpgxResult createUiLabel(
	UserInterface ui,
	const uint32_t* string,
	size_t stringLength,
	AlignmentType alignment,
	Vec3F position,
	cmmt_float_t scale,
	SrgbColor color,
	bool isBold,
	bool isItalic,
	bool useTags,
	bool isConstant,
	bool isUniversal,
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
 * string - label string or NULL.
 * stringLength - label string length or 0.
 * alignment - alignment type.
 * position - label position.
 * scale - label scale.
 * color - initial label color.
 * isBold - is label bold initially.
 * isItalic - is label italic initially.
 * useTags - use HTML tags.
 * isConstant - is label constant.
 * isUniversal - support any UTF character (Heavy).
 * parent - parent instance or NULL.
 * events - interface events or NULL.
 * handle - label handle or NULL.
 * isActive - is label active.
 * uiPanel - pointer to the UI label.
 */
MpgxResult createUiLabel8(
	UserInterface ui,
	const char* string,
	size_t stringLength,
	AlignmentType alignment,
	Vec3F position,
	cmmt_float_t scale,
	SrgbColor color,
	bool isBold,
	bool isItalic,
	bool useTags,
	bool isConstant,
	bool isUniversal,
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
 * Returns UI label text UTF-32 string.
 * label - UI label instance.
 */
const uint32_t* getUiLabelText(
	InterfaceElement label);
/*
 * Returns UI label text string length.
 * label - UI label instance.
 */
size_t getUiLabelTextLength(
	InterfaceElement label);

/*
 * Set UI label text UTF-32 string.
 * Returns operation MPGX result.
 *
 * label - UI label instance.
 * string - text string or NULL.
 * length - string length or 0.
 */
MpgxResult setUiLabelText(
	InterfaceElement label,
	const uint32_t* string,
	size_t length);
/*
 * Set UI label text UTF-8 string.
 * Returns operation MPGX result.
 *
 * label - UI label instance.
 * string - text string or NULL.
 * length - string length or 0.
 */
MpgxResult setUiLabelText8(
	InterfaceElement label,
	const char* string,
	size_t length);

/*
 * Create a new UTF-32 UI window instance.
 * Returns operation MPGX result.
 *
 * ui - user interface instance.
 * title - title string or NULL.
 * titleLength - title string length or 0.
 * alignment - alignment type.
 * position - window position.
 * scale - window scale.
 * parent - parent instance or NULL.
 * events - interface events or NULL.
 * handle - window handle or NULL.
 * isActive - is window active.
 * uiPanel - pointer to the UI window.
 */
MpgxResult createUiWindow(
	UserInterface ui,
	const uint32_t* title,
	size_t titleLength,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
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
 * title - title string or NULL.
 * titleLength - title string length or 0.
 * alignment - alignment type.
 * position - window position.
 * scale - window scale.
 * parent - parent instance or NULL.
 * events - interface events or NULL.
 * handle - window handle or NULL.
 * isActive - is window active.
 * uiPanel - pointer to the UI window.
 */
MpgxResult createUiWindow8(
	UserInterface ui,
	const char* title,
	size_t titleLength,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
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
 * Returns UI window panel render instance.
 * window - UI window instance.
 */
GraphicsRender getUiWindowPanelRender(InterfaceElement window);
/*
 * Returns UI window bar panel render instance.
 * window - UI window instance.
 */
GraphicsRender getUiWindowBarRender(InterfaceElement window);
/*
 * Returns UI window title text render instance.
 * window - UI window instance.
 */
GraphicsRender getUiWindowTitleRender(InterfaceElement window);
/*
 * Returns UI window on update event function.
 * window - UI window instance.
 */
OnInterfaceElementEvent getUiWindowOnUpdateEvent(InterfaceElement window);
/*
 * Returns UI window on press event function.
 * window - UI window instance.
 */
OnInterfaceElementEvent getUiWindowOnPressEvent(InterfaceElement window);

/*
 * Create a new UTF-32 UI button instance.
 * Returns operation MPGX result.
 *
 * ui - user interface instance.
 * text - text string or NULL.
 * textLength - text string length or 0.
 * alignment - alignment type.
 * position - button position.
 * scale - button scale.
 * isEnabled - is button enabled.
 * parent - parent instance or NULL.
 * events - interface events or NULL.
 * handle - button handle or NULL.
 * isActive - is button active.
 * uiButton - pointer to the UI button.
 */
MpgxResult createUiButton(
	UserInterface ui,
	const uint32_t* text,
	size_t textLength,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	bool isEnabled,
	Transform parent,
	const InterfaceElementEvents* events,
	void* handle,
	bool isActive,
	InterfaceElement* uiButton);
/*
 * Create a new UTF-8 UI button instance.
 * Returns operation MPGX result.
 *
 * ui - user interface instance.
 * text - text string or NULL.
 * textLength - text string length or 0.
 * alignment - alignment type.
 * position - button position.
 * scale - button scale.
 * isEnabled - is button enabled.
 * parent - parent instance or NULL.
 * events - interface events or NULL.
 * handle - button handle or NULL.
 * isActive - is button active.
 * uiButton - pointer to the UI button.
 */
MpgxResult createUiButton8(
	UserInterface ui,
	const char* text,
	size_t textLength,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	bool isEnabled,
	Transform parent,
	const InterfaceElementEvents* events,
	void* handle,
	bool isActive,
	InterfaceElement* uiButton);

/*
 * Returns UI button handle.
 * button - UI button instance.
 */
void* getUiButtonHandle(InterfaceElement button);
/*
 * Returns UI button panel render instance.
 * button - UI button instance.
 */
GraphicsRender getUiButtonPanelRender(InterfaceElement button);
/*
 * Returns UI button text render instance.
 * button - UI button instance.
 */
GraphicsRender getUiButtonTextRender(InterfaceElement button);
/*
 * Returns UI button on enable event function.
 * button - UI button instance.
 */
OnInterfaceElementEvent getUiButtonOnEnableEvent(InterfaceElement button);
/*
 * Returns UI button on disable event function.
 * button - UI button instance.
 */
OnInterfaceElementEvent getUiButtonOnDisableEvent(InterfaceElement button);
/*
 * Returns UI button on enter event function.
 * button - UI button instance.
 */
OnInterfaceElementEvent getUiButtonOnEnterEvent(InterfaceElement button);
/*
 * Returns UI button on exit event function.
 * button - UI button instance.
 */
OnInterfaceElementEvent getUiButtonOnExitEvent(InterfaceElement button);
/*
 * Returns UI button on press event function.
 * button - UI button instance.
 */
OnInterfaceElementEvent getUiButtonOnPressEvent(InterfaceElement button);
/*
 * Returns UI button on release event function.
 * button - UI button instance.
 */
OnInterfaceElementEvent getUiButtonOnReleaseEvent(InterfaceElement button);

/*
 * Returns UI button disabled color.
 * button - UI button instance.
 */
LinearColor getUiButtonDisabledColor(
	InterfaceElement button);
/*
 * Sets UI button disabled color.
 *
 * button - UI button instance.
 * color - color value.
 */
void setUiButtonDisabledColor(
	InterfaceElement button,
	LinearColor color);

/*
 * Returns UI button enabled color.
 * button - UI button instance.
 */
LinearColor getUiButtonEnabledColor(
	InterfaceElement button);
/*
 * Sets UI button enabled color.
 *
 * button - UI button instance.
 * color - color value.
 */
void setUiButtonEnabledColor(
	InterfaceElement button,
	LinearColor color);

/*
 * Returns UI button hovered color.
 * button - UI button instance.
 */
LinearColor getUiButtonHoveredColor(
	InterfaceElement button);
/*
 * Sets UI button hovered color.
 *
 * button - UI button instance.
 * color - color value.
 */
void setUiButtonHoveredColor(
	InterfaceElement button,
	LinearColor color);

/*
 * Returns UI button pressed color.
 * button - UI button instance.
 */
LinearColor getUiButtonPressedColor(
	InterfaceElement button);
/*
 * Sets UI button pressed color.
 *
 * button - UI button instance.
 * color - color value.
 */
void setUiButtonPressedColor(
	InterfaceElement button,
	LinearColor color);

/*
 * Create a new UTF-32 UI input field instance.
 * Returns operation MPGX result.
 *
 * ui - user interface instance.
 * placeholder - placeholder string or NULL.
 * placeholderLength - placeholder string length or 0.
 * alignment - alignment type.
 * position - input field position.
 * scale - input field scale.
 * maxLength - max input string length.
 * mask - password mask character or 0.
 * isEnabled - is input field enabled.
 * parent - parent instance or NULL.
 * events - interface events or NULL.
 * onChange - on change event function or NULL.
 * onDefocus - on defocus event function or NULL.
 * handle - input field handle or NULL.
 * isActive - is input field active.
 * uiInputField - pointer to the UI input field.
 */
MpgxResult createUiInputField(
	UserInterface ui,
	const uint32_t* placeholder,
	size_t placeholderLength,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	size_t maxLength,
	uint32_t mask,
	bool isEnabled,
	Transform parent,
	const InterfaceElementEvents* events,
	OnInterfaceElementEvent onChange,
	OnInterfaceElementEvent onDefocus,
	void* handle,
	bool isActive,
	InterfaceElement* uiInputField);
/*
 * Create a new UTF-8 UI input field instance.
 * Returns operation MPGX result.
 *
 * ui - user interface instance.
 * placeholder - placeholder string or NULL.
 * placeholderLength - placeholder string length or 0.
 * alignment - alignment type.
 * position - input field position.
 * scale - input field scale.
 * maxLength - max input string length.
 * mask - password mask character or 0.
 * isEnabled - is input field enabled.
 * parent - parent instance or NULL.
 * events - interface events or NULL.
 * onChange - on change event function or NULL.
 * onDefocus - on defocus event function or NULL.
 * handle - input field handle or NULL.
 * isActive - is input field active.
 * uiInputField - pointer to the UI input field.
 */
MpgxResult createUiInputField8(
	UserInterface ui,
	const char* placeholder,
	size_t placeholderLength,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	size_t maxLength,
	uint32_t mask,
	bool isEnabled,
	Transform parent,
	const InterfaceElementEvents* events,
	OnInterfaceElementEvent onChange,
	OnInterfaceElementEvent onDefocus,
	void* handle,
	bool isActive,
	InterfaceElement* uiInputField);

/*
 * Returns UI input field handle.
 * inputField - UI input field instance.
 */
void* getUiInputFieldHandle(InterfaceElement inputField);
/*
 * Returns UI input field panel render instance.
 * inputField - UI input field instance.
 */
GraphicsRender getUiInputFieldPanelRender(InterfaceElement inputField);
/*
 * Returns UI input field focus render instance.
 * inputField - UI input field instance.
 */
GraphicsRender getUiInputFieldFocusRender(InterfaceElement inputField);
/*
 * Returns UI input field text render instance.
 * inputField - UI input field instance.
 */
GraphicsRender getUiInputFieldTextRender(InterfaceElement inputField);
/*
 * Returns UI input field placeholder render instance.
 * inputField - UI input field instance.
 */
GraphicsRender getUiInputFieldPlaceholderRender(InterfaceElement inputField);
/*
 * Returns UI input field on enable event function.
 * inputField - UI input field instance.
 */
OnInterfaceElementEvent getUiInputFieldOnEnableEvent(InterfaceElement inputField);
/*
 * Returns UI input field on disable event function.
 * inputField - UI input field instance.
 */
OnInterfaceElementEvent getUiInputFieldOnDisableEvent(InterfaceElement inputField);
/*
 * Returns UI input field on enter event function.
 * inputField - UI input field instance.
 */
OnInterfaceElementEvent getUiInputFieldOnEnterEvent(InterfaceElement inputField);
/*
 * Returns UI input field on exit event function.
 * inputField - UI input field instance.
 */
OnInterfaceElementEvent getUiInputFieldOnExitEvent(InterfaceElement inputField);
/*
 * Returns UI input field on press event function.
 * inputField - UI input field instance.
 */
OnInterfaceElementEvent getUiInputFieldOnPressEvent(InterfaceElement inputField);
/*
 * Returns UI input field on change event function.
 * inputField - UI input field instance.
 */
OnInterfaceElementEvent getUiInputFieldOnChangeEvent(InterfaceElement inputField);
/*
 * Returns UI input field on defocus event function.
 * inputField - UI input field instance.
 */
OnInterfaceElementEvent getUiInputFieldOnDefocusEvent(InterfaceElement inputField);
/*
 * Returns UI input field maximal string length.
 * inputField - UI input field instance.
 */
size_t getUiInputFieldMaxLength(InterfaceElement inputField);
/*
 * Returns true if input field is currently focused.
 * inputField - UI input field instance.
 */
bool isUiInputFieldFocused(InterfaceElement inputField);

// TODO: focusUiInputField(InterfaceElement inputField);

/*
 * Returns UI input field disabled color.
 * inputField - UI input field instance.
 */
LinearColor getUiInputFieldDisabledColor(
	InterfaceElement inputField);
/*
 * Sets UI input field disabled color.
 *
 * inputField - UI input field instance.
 * color - color value.
 */
void setUiInputFieldDisabledColor(
	InterfaceElement inputField,
	LinearColor color);

/*
 * Returns UI input field enabled color.
 * inputField - UI input field instance.
 */
LinearColor getUiInputFieldEnabledColor(
	InterfaceElement inputField);
/*
 * Sets UI input field enabled color.
 *
 * inputField - UI input field instance.
 * color - color value.
 */
void setUiInputFieldEnabledColor(
	InterfaceElement inputField,
	LinearColor color);

/*
 * Returns UI input field focused color.
 * inputField - UI input field instance.
 */
LinearColor getUiInputFieldFocusedColor(
	InterfaceElement inputField);
/*
 * Sets UI input field focused color.
 *
 * inputField - UI input field instance.
 * color - color value.
 */
void setUiInputFieldFocusedColor(
	InterfaceElement inputField,
	LinearColor color);

/*
 * Returns UI input field mask.
 * inputField - UI input field instance.
 */
uint32_t getUiInputFieldMask(
	InterfaceElement inputField);
/*
 * Set UI input field mask.
 * Returns operation MPGX result.
 *
 * inputField - UI input field instance.
 * mask - character mask.
 */
MpgxResult setUiInputFieldMask(
	InterfaceElement inputField,
	uint32_t mask);

/*
 * Returns UI input field text UTF-32 string.
 * inputField - UI input field instance.
 */
const uint32_t* getUiInputFieldText(
	InterfaceElement inputField);
/*
 * Returns UI input field text string length.
 * inputField - UI input field instance.
 */
size_t getUiInputFieldTextLength(
	InterfaceElement inputField);

/*
 * Set UI input field text UTF-32 string.
 * Returns operation MPGX result.
 *
 * inputField - UI input field instance.
 * string - text string or NULL.
 * length - string length or 0.
 */
MpgxResult setUiInputFieldText(
	InterfaceElement inputField,
	const uint32_t* string,
	size_t length);
/*
 * Set UI input field text UTF-8 string.
 * Returns operation MPGX result.
 *
 * inputField - UI input field instance.
 * string - text string or NULL.
 * length - string length or 0.
 */
MpgxResult setUiInputFieldText8(
	InterfaceElement inputField,
	const char* string,
	size_t length);

/*
 * Create a new UTF-32 UI checkbox instance.
 * Returns operation MPGX result.
 *
 * ui - user interface instance.
 * text - text string or NULL.
 * textLength - text string length or 0.
 * alignment - alignment type.
 * position - checkbox position.
 * scale - checkbox scale.
 * isChecked - is checkbox checked.
 * isEnabled - is checkbox enabled.
 * parent - parent instance or NULL.
 * events - interface events or NULL.
 * handle - checkbox handle or NULL.
 * isActive - is checkbox active.
 * uiCheckbox - pointer to the UI checkbox.
 */
MpgxResult createUiCheckbox(
	UserInterface ui,
	const uint32_t* text,
	size_t textLength,
	AlignmentType alignment,
	Vec3F position,
	cmmt_float_t scale,
	bool isChecked,
	bool isEnabled,
	Transform parent,
	const InterfaceElementEvents* events,
	void* handle,
	bool isActive,
	InterfaceElement* uiCheckbox);
/*
 * Create a new UTF-8 UI checkbox instance.
 * Returns operation MPGX result.
 *
 * ui - user interface instance.
 * text - text string or NULL.
 * textLength - text string length or 0.
 * alignment - alignment type.
 * position - checkbox position.
 * scale - checkbox scale.
 * isChecked - is checkbox checked.
 * isEnabled - is checkbox enabled.
 * parent - parent instance or NULL.
 * events - interface events or NULL.
 * handle - checkbox handle or NULL.
 * isActive - is checkbox active.
 * uiCheckbox - pointer to the UI checkbox.
 */
MpgxResult createUiCheckbox8(
	UserInterface ui,
	const char* text,
	size_t textLength,
	AlignmentType alignment,
	Vec3F position,
	cmmt_float_t scale,
	bool isChecked,
	bool isEnabled,
	Transform parent,
	const InterfaceElementEvents* events,
	void* handle,
	bool isActive,
	InterfaceElement* uiCheckbox);

/*
 * Returns UI checkbox handle.
 * checkbox - UI checkbox instance.
 */
void* getUiCheckboxHandle(InterfaceElement checkbox);
/*
 * Returns UI checkbox panel render instance.
 * checkbox - UI checkbox instance.
 */
GraphicsRender getUiCheckboxPanelRender(InterfaceElement checkbox);
/*
 * Returns UI checkbox focus render instance.
 * checkbox - UI checkbox instance.
 */
GraphicsRender getUiCheckboxFocusRender(InterfaceElement checkbox);
/*
 * Returns UI checkbox check render instance.
 * checkbox - UI checkbox instance.
 */
GraphicsRender getUiCheckboxCheckRender(InterfaceElement checkbox);
/*
 * Returns UI checkbox text render instance.
 * checkbox - UI checkbox instance.
 */
GraphicsRender getUiCheckboxTextRender(InterfaceElement checkbox);
/*
 * Returns UI checkbox on enable event function.
 * checkbox - UI checkbox instance.
 */
OnInterfaceElementEvent getUiCheckboxOnEnableEvent(InterfaceElement checkbox);
/*
 * Returns UI checkbox on disable event function.
 * checkbox - UI checkbox instance.
 */
OnInterfaceElementEvent getUiCheckboxOnDisableEvent(InterfaceElement checkbox);
/*
 * Returns UI checkbox on enter event function.
 * checkbox - UI checkbox instance.
 */
OnInterfaceElementEvent getUiCheckboxOnEnterEvent(InterfaceElement checkbox);
/*
 * Returns UI checkbox on exit event function.
 * checkbox - UI checkbox instance.
 */
OnInterfaceElementEvent getUiCheckboxOnExitEvent(InterfaceElement checkbox);
/*
 * Returns UI checkbox on press event function.
 * checkbox - UI checkbox instance.
 */
OnInterfaceElementEvent getUiCheckboxOnPressEvent(InterfaceElement checkbox);
/*
 * Returns UI checkbox on release event function.
 * checkbox - UI checkbox instance.
 */
OnInterfaceElementEvent getUiCheckboxOnReleaseEvent(InterfaceElement checkbox);

/*
 * Returns UI checkbox disabled color.
 * checkbox - UI checkbox instance.
 */
LinearColor getUiCheckboxDisabledColor(
	InterfaceElement checkbox);
/*
 * Sets UI checkbox disabled color.
 *
 * checkbox - UI checkbox instance.
 * color - color value.
 */
void setUiCheckboxDisabledColor(
	InterfaceElement checkbox,
	LinearColor color);

/*
 * Returns UI checkbox enabled color.
 * checkbox - UI button instance.
 */
LinearColor getUiCheckboxEnabledColor(
	InterfaceElement checkbox);
/*
 * Sets UI checkbox enabled color.
 *
 * checkbox - UI checkbox instance.
 * color - color value.
 */
void setUiCheckboxEnabledColor(
	InterfaceElement checkbox,
	LinearColor color);

/*
 * Returns UI checkbox hovered color.
 * checkbox - UI checkbox instance.
 */
LinearColor getUiCheckboxHoveredColor(
	InterfaceElement checkbox);
/*
 * Sets UI checkbox hovered color.
 *
 * checkbox - UI checkbox instance.
 * color - color value.
 */
void setUiCheckboxHoveredColor(
	InterfaceElement checkbox,
	LinearColor color);

/*
 * Returns UI checkbox pressed color.
 * checkbox - UI checkbox instance.
 */
LinearColor getUiCheckboxPressedColor(
	InterfaceElement checkbox);
/*
 * Sets UI checkbox pressed color.
 *
 * checkbox - UI checkbox instance.
 * color - color value.
 */
void setUiCheckboxPressedColor(
	InterfaceElement checkbox,
	LinearColor color);

/*
 * Returns true if UI checkbox is checked.
 * checkbox - UI checkbox instance.
 */
bool isCheckboxChecked(
	InterfaceElement checkbox);
/*
 * Sets UI checkbox checked value.
 *
 * checkbox - UI checkbox instance.
 * isChecked - is checked value.
 */
void setCheckboxChecked(
	InterfaceElement checkbox,
	bool isChecked);

// TODO: mask (only transform with masking)
// TODO: InputBox, ScrollBox
