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

#include "uran/user_interface.h"
#include <string.h>

#if _WIN32
#undef interface
#endif

#define UI_PANEL_NAME "Panel"
#define UI_LABEL_NAME "Label"
#define UI_WINDOW_NAME "Window"
#define UI_BUTTON_NAME "Button"
#define UI_INPUT_FIELD_NAME "InputField"

struct UserInterface_T
{
	Window window;
	Transformer transformer;
	FontAtlas fontAtlas;
	Interface interface;
	GraphicsRenderer panelRenderer;
	GraphicsRenderer textRenderer;
	GraphicsRender cursorRender;
	InterfaceElement focusedInputField;
	double blinkDelay;
	double buttonDelay;
	uint32_t cursorIndex;
	bool isMousePressed;
	bool isButtonPressed;
};

typedef struct UiPanelHandle_T
{
	void* handle;
	GraphicsRender render;
} UiPanelHandle_T;
typedef struct UiLabelHandle_T
{
	void* handle;
	GraphicsRender render;
} UiLabelHandle_T;
typedef struct UiWindowHandle_T
{
	UserInterface ui;
	void* handle;
	OnInterfaceElementEvent onUpdate;
	OnInterfaceElementEvent onPress;
	GraphicsRender barRender;
	GraphicsRender panelRender;
	GraphicsRender titleRender;
	Vec2F lastCursorPosition;
	bool isDragging;
} UiWindowHandle_T;
typedef struct UiButtonHandle_T
{
	UserInterface ui;
	void* handle;
	OnInterfaceElementEvent onEnable;
	OnInterfaceElementEvent onDisable;
	OnInterfaceElementEvent onEnter;
	OnInterfaceElementEvent onExit;
	OnInterfaceElementEvent onPress;
	OnInterfaceElementEvent onRelease;
	LinearColor disabledColor;
	LinearColor enabledColor;
	LinearColor hoveredColor;
	LinearColor pressedColor;
	GraphicsRender panelRender;
	GraphicsRender textRender;
	bool isPressed;
} UiButtonHandle_T;
typedef struct UiInputFieldHandle_T
{
	UserInterface ui;
	void* handle;
	OnInterfaceElementEvent onEnable;
	OnInterfaceElementEvent onDisable;
	OnInterfaceElementEvent onEnter;
	OnInterfaceElementEvent onExit;
	OnInterfaceElementEvent onPress;
	LinearColor disabledColor;
	LinearColor enabledColor;
	LinearColor focusedColor;
	size_t maxLength;
	GraphicsRender panelRender;
	GraphicsRender focusRender;
	GraphicsRender textRender;
	GraphicsRender placeholderRender;
} UiInputFieldHandle_T;

typedef UiPanelHandle_T* UiPanelHandle;
typedef UiLabelHandle_T* UiLabelHandle;
typedef UiWindowHandle_T* UiWindowHandle;
typedef UiButtonHandle_T* UiButtonHandle;
typedef UiInputFieldHandle_T* UiInputFieldHandle;

// TODO: replace UTF32 conversion with fast native text creation method

inline static GraphicsRender createCursorRenderInstance(
	Transformer transformer,
	GraphicsRenderer panelRenderer)
{
	assert(transformer);
	assert(panelRenderer);

	Transform transform = createTransform(
		transformer,
		zeroVec3F,
		oneVec3F,
		zeroQuat,
		NO_ROTATION_TYPE,
		NULL,
		false);

	if (!transform)
		return NULL;

	GraphicsRender render = createPanelRender(
		panelRenderer,
		transform,
		oneSizeBox3F,
		srgbToLinearColor(DEFAULT_UI_TEXT_COLOR));

	if (!render)
	{
		destroyTransform(transform);
		return NULL;
	}

	return render;
}
inline static void destroyCursorRenderInstance(
	GraphicsRender cursorRender)
{
	if (!cursorRender)
		return;

	Transform transform = getGraphicsRenderTransform(cursorRender);
	destroyGraphicsRender(cursorRender);
	destroyTransform(transform);
}
MpgxResult createUserInterface(
	Transformer transformer,
	GraphicsPipeline panelPipeline,
	GraphicsPipeline textPipeline,
	FontAtlas fontAtlas,
	cmmt_float_t scale,
	size_t capacity,
	ThreadPool threadPool,
	UserInterface* ui)
{
	assert(transformer);
	assert(panelPipeline);
	assert(textPipeline);
	assert(fontAtlas);
	assert(scale > 0.0f);
	assert(ui);

	Window window = getGraphicsPipelineWindow(panelPipeline);

	UserInterface userInterface = calloc(1,
		sizeof(UserInterface_T));

	if (!userInterface)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	userInterface->window = window;
	userInterface->transformer = transformer;
	userInterface->fontAtlas = fontAtlas;
	userInterface->focusedInputField = NULL;
	userInterface->blinkDelay = 0.0;
	userInterface->buttonDelay = 0.0;
	userInterface->cursorIndex = 0;
	userInterface->isMousePressed = false;
	userInterface->isButtonPressed = false;

	Interface interface = createInterface(
		window,
		scale,
		capacity,
		threadPool);

	if (!interface)
	{
		destroyUserInterface(userInterface);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	userInterface->interface = interface;

	GraphicsRenderer panelRenderer = createPanelRenderer(
		panelPipeline,
		DESCENDING_GRAPHICS_RENDER_SORTING,
		false,
		1,
		threadPool);

	if (!panelRenderer)
	{
		destroyUserInterface(userInterface);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	userInterface->panelRenderer = panelRenderer;

	GraphicsRenderer textRenderer = createTextRenderer(
		textPipeline,
		DESCENDING_GRAPHICS_RENDER_SORTING,
		false,
		1,
		threadPool);

	if (!textRenderer)
	{
		destroyUserInterface(userInterface);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	userInterface->textRenderer = textRenderer;

	GraphicsRender cursorRender = createCursorRenderInstance(
		transformer,
		panelRenderer);

	if (!cursorRender)
	{
		destroyUserInterface(userInterface);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	userInterface->cursorRender = cursorRender;

	*ui = userInterface;
	return SUCCESS_MPGX_RESULT;
}
void destroyUserInterface(UserInterface ui)
{
	if (!ui)
		return;

	destroyCursorRenderInstance(ui->cursorRender);
	destroyGraphicsRenderer(ui->textRenderer);
	destroyGraphicsRenderer(ui->panelRenderer);
	destroyInterface(ui->interface);
	free(ui);
}

Transformer getUserInterfaceTransformer(UserInterface ui)
{
	assert(ui);
	return ui->transformer;
}
GraphicsPipeline getUserInterfacePanelPipeline(UserInterface ui)
{
	assert(ui);
	return getGraphicsRendererPipeline(ui->panelRenderer);
}
GraphicsPipeline getUserInterfaceTextPipeline(UserInterface ui)
{
	assert(ui);
	return getGraphicsRendererPipeline(ui->textRenderer);
}
FontAtlas getUserInterfaceFontAtlas(UserInterface ui)
{
	assert(ui);
	return ui->fontAtlas;
}
Interface getUserInterface(UserInterface ui)
{
	assert(ui);
	return ui->interface;
}
GraphicsRender getUserInterfaceCursor(UserInterface ui)
{
	assert(ui);
	return ui->cursorRender;
}

inline static void updateUiCursor(
	UserInterface ui,
	Transform textTransform,
	Text text)
{
	assert(ui);
	assert(textTransform);
	assert(text);

	Transform cursorTransform = getGraphicsRenderTransform(ui->cursorRender);
	Vec3F textPosition = getTranslationMat4F(
		getTransformModel(textTransform));
	Vec3F textScale = getTransformScale(textTransform);
	Vec3F cursorScale = getTransformScale(cursorTransform);

	Vec2F cursorOffset;

	if (getTextLength(text) > 0)
	{
		bool result = getTextCursorAdvance(text,
			ui->cursorIndex,
			&cursorOffset);

		if (result)
		{
			cursorOffset.x *= textScale.x;
			cursorOffset.y *= textScale.y;
		}
		else
		{
			cursorOffset = zeroVec2F;
		}
	}
	else
	{
		cursorOffset = zeroVec2F;
	}

	setTransformPosition(cursorTransform, vec3F(
		textPosition.x + cursorOffset.x,
		textPosition.y + cursorOffset.y,
		textPosition.z));
	setTransformScale(cursorTransform, vec3F(
		cursorScale.x,
		getTransformScale(textTransform).y * (cmmt_float_t)1.25,
		(cmmt_float_t)1.0));
	setTransformActive(cursorTransform, true);
	ui->blinkDelay = getWindowUpdateTime(ui->window) + 0.5;
}
inline static void updateUiInputFields(UserInterface ui)
{
	assert(ui);
	Window window = ui->window;

	if (getWindowMouseButton(window, LEFT_MOUSE_BUTTON))
	{
		if (!ui->isMousePressed)
		{
			if (ui->focusedInputField)
			{
				UiInputFieldHandle handle = getInterfaceElementHandle(
					ui->focusedInputField);
				setPanelRenderColor(
					handle->focusRender,
					handle->enabledColor);
				Transform cursorTransform = getGraphicsRenderTransform(
					ui->cursorRender);
				setTransformActive(
					cursorTransform,
					false);
				ui->focusedInputField = NULL;
			}

			ui->isMousePressed = true;
		}
	}
	else
	{
		ui->isMousePressed = false;
	}

	if (ui->focusedInputField)
	{
		UiInputFieldHandle handle = getInterfaceElementHandle(
			ui->focusedInputField);
		Text text = getTextRenderText(handle->textRender);
		double updateTime = getWindowUpdateTime(window);

		bool isTextChanged = false, isCursorChanged = false;
		// TODO: custom delays and cursor color

		if ((getWindowKeyboardKey(window, BACKSPACE_KEYBOARD_KEY) ||
			getWindowKeyboardKey(window, DELETE_KEYBOARD_KEY)))
		{
			if (getTextLength(text) > 0 && ui->cursorIndex > 0 &&
				(!ui->isButtonPressed || ui->buttonDelay < updateTime))
			{
				ui->cursorIndex--;
				removeTextChar(text, ui->cursorIndex);
				ui->buttonDelay = ui->isButtonPressed ?
					updateTime + 0.1 : updateTime + 0.3;
				ui->isButtonPressed = isTextChanged = isCursorChanged = true;
			}
		}
		else if (getWindowKeyboardKey(window, LEFT_KEYBOARD_KEY))
		{
			if (ui->cursorIndex > 0 &&
				(!ui->isButtonPressed || ui->buttonDelay < updateTime))
			{
				ui->cursorIndex--;
				ui->buttonDelay = ui->isButtonPressed ?
					updateTime + 0.1 : updateTime + 0.3;
				ui->isButtonPressed = isCursorChanged = true;
			}
		}
		else if (getWindowKeyboardKey(window, RIGHT_KEYBOARD_KEY))
		{
			if (ui->cursorIndex < getTextLength(text) &&
				(!ui->isButtonPressed || ui->buttonDelay < updateTime))
			{
				ui->cursorIndex++;
				ui->buttonDelay = ui->isButtonPressed ?
					updateTime + 0.1 : updateTime + 0.3;
				ui->isButtonPressed = isCursorChanged = true;
			}
		}
		else
		{
			ui->isButtonPressed = false;
		}

		size_t inputLength = getWindowInputLength(window);

		if (getTextLength(text) + inputLength > handle->maxLength)
			inputLength = handle->maxLength - getTextLength(text);

		if (inputLength > 0)
		{
			bool result = appendTextString32(text,
				getWindowInputBuffer(window),
				inputLength,
				ui->cursorIndex);

			if (result)
			{
				ui->cursorIndex += inputLength;
				isTextChanged = isCursorChanged = true;
			}
		}

		if (isTextChanged)
		{
			if (getTextLength(text) == 0)
			{
				setTransformActive(getGraphicsRenderTransform(
					handle->textRender), false);
				setTransformActive(getGraphicsRenderTransform(
					handle->placeholderRender), true);
			}
			else
			{
				setTransformActive(getGraphicsRenderTransform(
					handle->textRender), true);
				setTransformActive(getGraphicsRenderTransform(
					handle->placeholderRender), false);
				MpgxResult mpgxResult = bakeText(text);
				if (mpgxResult != SUCCESS_MPGX_RESULT) isCursorChanged = false;
			}
		}
		if (isCursorChanged)
		{
			Transform textTransform = getGraphicsRenderTransform(
				handle->placeholderRender);
			updateUiCursor(ui, textTransform, text);
		}

		if (updateTime > ui->blinkDelay)
		{
			Transform cursorTransform = getGraphicsRenderTransform(
				ui->cursorRender);
			setTransformActive(
				cursorTransform,
				!isTransformActive(cursorTransform));
			ui->blinkDelay = updateTime + 0.5;
		}
	}
}
void updateUserInterface(UserInterface ui)
{
	assert(ui);
	updateUiInputFields(ui);
	updateInterface(ui->interface);
}
GraphicsRendererResult drawUserInterface(UserInterface ui)
{
	assert(ui);

	GraphicsRendererResult result =
		createGraphicsRendererResult();
	GraphicsRendererResult tmpResult;

	Mat4F view = translateMat4F(identMat4F, vec3F(
		(cmmt_float_t)0.0, (cmmt_float_t)0.0, (cmmt_float_t)0.5));
	Camera camera = createInterfaceCamera(ui->interface);

	GraphicsRendererData data = createGraphicsRenderData(
		view, camera, false);

	tmpResult = drawGraphicsRenderer(ui->panelRenderer, &data);
	result = addGraphicsRendererResult(result, tmpResult);

	tmpResult = drawGraphicsRenderer(ui->textRenderer, &data);
	result = addGraphicsRendererResult(result, tmpResult);
	return result;
}

static void onUiPanelDestroy(void* _handle)
{
	assert(_handle);

	UiPanelHandle handle = (UiPanelHandle)_handle;
	GraphicsRender render = handle->render;

	if (render)
	{
		Transform transform = getGraphicsRenderTransform(render);
		destroyGraphicsRender(render);
		destroyTransform(transform);
	}

	free(handle);
}
MpgxResult createUiPanel(
	UserInterface ui,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	Transform parent,
	const InterfaceElementEvents* events,
	void* _handle,
	bool isActive,
	InterfaceElement* uiPanel)
{
	assert(ui);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(scale.x > 0.0f);
	assert(scale.y > 0.0f);
	assert(uiPanel);

	assert(!parent || (parent && ui->transformer ==
		getTransformTransformer(parent)));

	UiPanelHandle handle = calloc(1,
		sizeof(UiPanelHandle_T));

	if (!handle)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	handle->handle = _handle;

	Transform transform = createTransform(
		ui->transformer,
		zeroVec3F,
		vec3F(scale.x, scale.y, (cmmt_float_t)1.0),
		oneQuat,
		NO_ROTATION_TYPE,
		parent,
		isActive);

	if (!transform)
	{
		onUiPanelDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	GraphicsRender render = createPanelRender(
		ui->panelRenderer,
		transform,
		oneSizeBox3F,
		srgbToLinearColor(DEFAULT_UI_PANEL_COLOR));

	if (!render)
	{
		destroyTransform(transform);
		onUiPanelDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	handle->render = render;

#ifndef NDEBUG
	const char* name = UI_PANEL_NAME;
#else
	const char* name = NULL;
#endif

	InterfaceElement element = createInterfaceElement(
		ui->interface,
		name,
		transform,
		alignment,
		position,
		oneSizeBox2F,
		events ? true : false,
		onUiPanelDestroy,
		events,
		handle);

	if (!element)
	{
		onUiPanelDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	*uiPanel = element;
	return SUCCESS_MPGX_RESULT;
}

void* getUiPanelHandle(InterfaceElement panel)
{
	assert(panel);
	assert(strcmp(getInterfaceElementName(
		panel), UI_PANEL_NAME) == 0);
	UiPanelHandle handle =
		getInterfaceElementHandle(panel);
	return handle->handle;
}
GraphicsRender getUiPanelRender(InterfaceElement panel)
{
	assert(panel);
	assert(strcmp(getInterfaceElementName(
		panel), UI_PANEL_NAME) == 0);
	UiPanelHandle handle =
		getInterfaceElementHandle(panel);
	return handle->render;
}

static void onUiLabelDestroy(void* _handle)
{
	assert(_handle);

	UiLabelHandle handle = (UiLabelHandle)_handle;
	GraphicsRender render = handle->render;

	if (render)
	{
		Text text = getTextRenderText(render);
		Transform transform = getGraphicsRenderTransform(render);
		destroyGraphicsRender(render);
		destroyText(text);
		destroyTransform(transform);
	}

	free(handle);
}
MpgxResult createUiLabel(
	UserInterface ui,
	const uint32_t* string,
	size_t stringLength,
	AlignmentType alignment,
	Vec3F position,
	cmmt_float_t scale,
	SrgbColor color,
	bool useTags,
	bool isConstant,
	Transform parent,
	const InterfaceElementEvents* events,
	void* _handle,
	bool isActive,
	InterfaceElement* uiLabel)
{
	assert(ui);
	assert(string);
	assert(stringLength > 0);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(scale > 0.0f);
	assert(uiLabel);

	assert(!parent || (parent && ui->transformer ==
		getTransformTransformer(parent)));

	UiLabelHandle handle = calloc(1,
		sizeof(UiButtonHandle_T));

	if (!handle)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	handle->handle = _handle;

	Transform transform = createTransform(
		ui->transformer,
		zeroVec3F,
		vec3F(scale, scale, (cmmt_float_t)1.0),
		oneQuat,
		NO_ROTATION_TYPE,
		parent,
		isActive);

	if (!transform)
	{
		onUiLabelDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	Text text;

	MpgxResult mpgxResult = createAtlasText(
		ui->fontAtlas,
		string,
		stringLength,
		alignment,
		color,
		useTags,
		isConstant,
		&text);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyTransform(transform);
		onUiLabelDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	Vec2F textSize = getTextSize(text);

	GraphicsRender render = createTextRender(
		ui->textRenderer,
		transform,
		createTextBox3F(alignment, textSize),
		whiteLinearColor,
		text,
		zeroVec4I);

	if (!render)
	{
		destroyText(text);
		destroyTransform(transform);
		onUiLabelDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	handle->render = render;

#ifndef NDEBUG
	const char* name = UI_LABEL_NAME;
#else
	const char* name = NULL;
#endif

	InterfaceElement element = createInterfaceElement(
		ui->interface,
		name,
		transform,
		alignment,
		position,
		createTextBox2F(alignment, textSize),
		events ? true : false,
		onUiLabelDestroy,
		events,
		handle);

	if (!element)
	{
		onUiLabelDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	*uiLabel = element;
	return SUCCESS_MPGX_RESULT;
}
MpgxResult createUiLabel8(
	UserInterface ui,
	const char* string,
	size_t stringLength,
	AlignmentType alignment,
	Vec3F position,
	cmmt_float_t scale,
	SrgbColor color,
	bool useTags,
	bool isConstant,
	Transform parent,
	const InterfaceElementEvents* events,
	void* handle,
	bool isActive,
	InterfaceElement* uiLabel)
{
	assert(ui);
	assert(string);
	assert(stringLength > 0);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(scale > 0.0f);
	assert(uiLabel);

	assert(!parent || (parent && ui->transformer ==
		getTransformTransformer(parent)));

	uint32_t* string32;
	size_t stringLength32;

	MpgxResult mpgxResult = allocateStringUTF32(
		string,
		stringLength,
		&string32,
		&stringLength32);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	mpgxResult = createUiLabel(
		ui,
		string32,
		stringLength32,
		alignment,
		position,
		scale,
		color,
		useTags,
		isConstant,
		parent,
		events,
		handle,
		isActive,
		uiLabel);

	free(string32);
	return mpgxResult;
}

void* getUiLabelHandle(InterfaceElement label)
{
	assert(label);
	assert(strcmp(getInterfaceElementName(
		label), UI_LABEL_NAME) == 0);
	UiLabelHandle handle =
		getInterfaceElementHandle(label);
	return handle->handle;
}
GraphicsRender getUiLabelRender(InterfaceElement label)
{
	assert(label);
	assert(strcmp(getInterfaceElementName(
		label), UI_LABEL_NAME) == 0);
	UiLabelHandle handle =
		getInterfaceElementHandle(label);
	return handle->render;
}

static void onUiWindowPress(InterfaceElement element)
{
	assert(element);

	UiWindowHandle handle = (UiWindowHandle)
		getInterfaceElementHandle(element);

	if (!handle->isDragging)
	{
		handle->lastCursorPosition =
			getWindowCursorPosition(handle->ui->window);
		handle->isDragging = true;
	}

	if (handle->onPress)
		handle->onPress(element);
}
static void onUiWindowUpdate(InterfaceElement element)
{
	assert(element);

	UiWindowHandle handle = (UiWindowHandle)
		getInterfaceElementHandle(element);

	if (handle->isDragging)
	{
		Window window = handle->ui->window;

		if (!getWindowMouseButton(window, LEFT_MOUSE_BUTTON))
		{
			handle->isDragging = false;
			return;
		}

		Vec2F cursorPosition = getWindowCursorPosition(window);
		Vec2F offset = subVec2F(cursorPosition, handle->lastCursorPosition);
		Vec3F position = getInterfaceElementPosition(element);
		cmmt_float_t scale = getInterfaceScale(handle->ui->interface);

		position.x += offset.x / scale;
		position.y -= offset.y / scale;

		setInterfaceElementPosition(element, position);
		handle->lastCursorPosition = cursorPosition;
	}

	if (handle->onUpdate)
		handle->onUpdate(element);
}
static void onUiWindowDestroy(void* _handle)
{
	assert(_handle);
	UiWindowHandle handle = (UiWindowHandle)_handle;

	Transform transform;
	GraphicsRender render = handle->titleRender;

	if (render)
	{
		Text text = getTextRenderText(render);
		transform = getGraphicsRenderTransform(render);
		destroyGraphicsRender(render);
		destroyText(text);
		destroyTransform(transform);
	}

	render = handle->panelRender;

	if (render)
	{
		transform = getGraphicsRenderTransform(render);
		destroyGraphicsRender(render);
		destroyTransform(transform);
	}

	render = handle->barRender;

	if (render)
	{
		transform = getGraphicsRenderTransform(render);
		destroyGraphicsRender(render);
		destroyTransform(transform);
	}

	free(handle);
}
MpgxResult createUiWindow(
	UserInterface ui,
	const uint32_t* title,
	size_t titleLength,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	SrgbColor titleColor,
	Transform parent,
	const InterfaceElementEvents* events,
	void* _handle,
	bool isActive,
	InterfaceElement* uiWindow)
{
	assert(ui);
	assert(title);
	assert(titleLength > 0);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(scale.x > 0.0f);
	assert(scale.y > 0.0f);
	assert(uiWindow);

	assert(!parent || (parent && ui->transformer ==
		getTransformTransformer(parent)));

	UiWindowHandle handle = calloc(1,
		sizeof(UiWindowHandle_T));

	if (!handle)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	handle->ui = ui;
	handle->handle = _handle;
	handle->lastCursorPosition = zeroVec2F;
	handle->isDragging = false;

	Transformer transformer = ui->transformer;
	GraphicsRenderer panelRenderer = ui->panelRenderer;

	Transform barTransform = createTransform(
		transformer,
		zeroVec3F,
		vec3F(
			scale.x,
			DEFAULT_UI_BAR_HEIGHT,
			(cmmt_float_t)1.0),
		oneQuat,
		NO_ROTATION_TYPE,
		parent,
		isActive);

	if (!barTransform)
	{
		onUiWindowDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	GraphicsRender barRender = createPanelRender(
		panelRenderer,
		barTransform,
		oneSizeBox3F,
		srgbToLinearColor(DEFAULT_UI_BAR_COLOR));

	if (!barRender)
	{
		destroyTransform(barTransform);
		onUiWindowDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	handle->barRender = barRender;

	Transform panelTransform = createTransform(
		transformer,
		vec3F(
			(cmmt_float_t)0.0,
			-(scale.y * (cmmt_float_t)0.5 +
				DEFAULT_UI_BAR_HEIGHT * (cmmt_float_t)0.5),
			(cmmt_float_t)0.0),
		vec3F(
			scale.x,
			scale.y,
			(cmmt_float_t)1.0),
		oneQuat,
		NO_ROTATION_TYPE,
		barTransform,
		true);

	if (!panelTransform)
	{
		onUiWindowDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	GraphicsRender panelRender = createPanelRender(
		panelRenderer,
		panelTransform,
		oneSizeBox3F,
		srgbToLinearColor(DEFAULT_UI_PANEL_COLOR));

	if (!panelRender)
	{
		destroyTransform(panelTransform);
		onUiWindowDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	handle->panelRender = panelRender;

	Transform titleTransform = createTransform(
		transformer,
		vec3F(
			(cmmt_float_t)0.0,
			(cmmt_float_t)0.0,
			(cmmt_float_t)-0.001),
		vec3F(
			DEFAULT_UI_TEXT_HEIGHT,
			DEFAULT_UI_TEXT_HEIGHT,
			(cmmt_float_t)1.0),
		oneQuat,
		NO_ROTATION_TYPE,
		barTransform,
		true);

	if (!titleTransform)
	{
		onUiWindowDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	Text text;

	MpgxResult mpgxResult = createAtlasText(
		ui->fontAtlas,
		title,
		titleLength,
		CENTER_ALIGNMENT_TYPE,
		titleColor,
		true,
		true,
		&text);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyTransform(titleTransform);
		onUiWindowDestroy(handle);
		return mpgxResult;
	}

	GraphicsRender titleRender = createTextRender(
		ui->textRenderer,
		titleTransform,
		createTextBox3F(
			alignment,
			getTextSize(text)),
		whiteLinearColor,
		text,
		zeroVec4I);

	if (!titleRender)
	{
		destroyText(text);
		destroyTransform(titleTransform);
		onUiWindowDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	handle->titleRender = titleRender;

#ifndef NDEBUG
	const char* name = UI_WINDOW_NAME;
#else
	const char* name = NULL;
#endif

	InterfaceElementEvents elementEvents = events ?
		*events : emptyInterfaceElementEvents;
	handle->onUpdate = elementEvents.onUpdate;
	handle->onPress = elementEvents.onPress;
	elementEvents.onUpdate = onUiWindowUpdate;
	elementEvents.onPress = onUiWindowPress;

	InterfaceElement element = createInterfaceElement(
		ui->interface,
		name,
		barTransform,
		alignment,
		position,
		oneSizeBox2F,
		true,
		onUiWindowDestroy,
		&elementEvents,
		handle);

	if (!element)
	{
		onUiWindowDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	*uiWindow = element;
	return SUCCESS_MPGX_RESULT;
}
MpgxResult createUiWindow8(
	UserInterface ui,
	const char* title,
	size_t titleLength,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	SrgbColor titleColor,
	Transform parent,
	const InterfaceElementEvents* events,
	void* handle,
	bool isActive,
	InterfaceElement* uiWindow)
{
	assert(ui);
	assert(title);
	assert(titleLength > 0);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(scale.x > 0.0f);
	assert(scale.y > 0.0f);
	assert(uiWindow);

	assert(!parent || (parent && ui->transformer ==
		getTransformTransformer(parent)));

	uint32_t* title32;
	size_t titleLength32;

	MpgxResult mpgxResult = allocateStringUTF32(
		title,
		titleLength,
		&title32,
		&titleLength32);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	mpgxResult = createUiWindow(
		ui,
		title32,
		titleLength32,
		alignment,
		position,
		scale,
		titleColor,
		parent,
		events,
		handle,
		isActive,
		uiWindow);

	free(title32);
	return mpgxResult;
}

void* getUiWindowHandle(InterfaceElement window)
{
	assert(window);
	assert(strcmp(getInterfaceElementName(
		window), UI_WINDOW_NAME) == 0);
	UiWindowHandle handle =
		getInterfaceElementHandle(window);
	return handle->handle;
}
GraphicsRender getUiWindowPanelRender(InterfaceElement window)
{
	assert(window);
	assert(strcmp(getInterfaceElementName(
		window), UI_WINDOW_NAME) == 0);
	UiWindowHandle handle =
		getInterfaceElementHandle(window);
	return handle->panelRender;
}
GraphicsRender getUiWindowBarRender(InterfaceElement window)
{
	assert(window);
	assert(strcmp(getInterfaceElementName(
		window), UI_WINDOW_NAME) == 0);
	UiWindowHandle handle =
		getInterfaceElementHandle(window);
	return handle->barRender;
}
GraphicsRender getUiWindowTitleRender(InterfaceElement window)
{
	assert(window);
	assert(strcmp(getInterfaceElementName(
		window), UI_WINDOW_NAME) == 0);
	UiWindowHandle handle =
		getInterfaceElementHandle(window);
	return handle->titleRender;
}
OnInterfaceElementEvent getUiWindowOnUpdateEvent(InterfaceElement window)
{
	assert(window);
	assert(strcmp(getInterfaceElementName(
		window), UI_WINDOW_NAME) == 0);
	UiWindowHandle handle =
		getInterfaceElementHandle(window);
	return handle->onUpdate;
}
OnInterfaceElementEvent getUiWindowOnPressEvent(InterfaceElement window)
{
	assert(window);
	assert(strcmp(getInterfaceElementName(
		window), UI_WINDOW_NAME) == 0);
	UiWindowHandle handle =
		getInterfaceElementHandle(window);
	return handle->onPress;
}

static void onUiButtonEnable(InterfaceElement element)
{
	assert(element);
	UiButtonHandle handle = (UiButtonHandle)
		getInterfaceElementHandle(element);
	setPanelRenderColor(
		handle->panelRender,
		handle->enabledColor);
	if (handle->onEnable)
		handle->onEnable(element);
}
static void onUiButtonDisable(InterfaceElement element)
{
	assert(element);
	UiButtonHandle handle = (UiButtonHandle)
		getInterfaceElementHandle(element);
	setPanelRenderColor(
		handle->panelRender,
		handle->disabledColor);
	if (handle->onDisable)
		handle->onDisable(element);
}
static void onUiButtonEnter(InterfaceElement element)
{
	assert(element);
	UiButtonHandle handle = (UiButtonHandle)
		getInterfaceElementHandle(element);
	setPanelRenderColor(
		handle->panelRender,
		handle->hoveredColor);
	if (handle->onEnter)
		handle->onEnter(element);
}
static void onUiButtonExit(InterfaceElement element)
{
	assert(element);
	UiButtonHandle handle = (UiButtonHandle)
		getInterfaceElementHandle(element);
	setPanelRenderColor(
		handle->panelRender,
		handle->enabledColor);
	handle->isPressed = false;
	if (handle->onExit)
		handle->onExit(element);
}
static void onUiButtonPress(InterfaceElement element)
{
	assert(element);
	UiButtonHandle handle = (UiButtonHandle)
		getInterfaceElementHandle(element);
	setPanelRenderColor(
		handle->panelRender,
		handle->pressedColor);
	handle->isPressed = true;
	if (handle->onPress)
		handle->onPress(element);
}
static void onUiButtonRelease(InterfaceElement element)
{
	assert(element);
	UiButtonHandle handle = (UiButtonHandle)
		getInterfaceElementHandle(element);
	setPanelRenderColor(
		handle->panelRender,
		handle->hoveredColor);
	if (handle->isPressed && handle->onRelease)
	{
		handle->isPressed = false;
		handle->onRelease(element);
		return;
	}
	handle->isPressed = false;
}
static void onUiButtonDestroy(void* _handle)
{
	assert(_handle);
	UiButtonHandle handle = (UiButtonHandle)_handle;

	Transform transform;
	GraphicsRender render = handle->textRender;

	if (render)
	{
		Text text = getTextRenderText(render);
		transform = getGraphicsRenderTransform(render);
		destroyGraphicsRender(render);
		destroyText(text);
		destroyTransform(transform);
	}

	render = handle->panelRender;

	if (render)
	{
		transform = getGraphicsRenderTransform(render);
		destroyGraphicsRender(render);
		destroyTransform(transform);
	}

	free(handle);
}
MpgxResult createUiButton(
	UserInterface ui,
	const uint32_t* text,
	size_t textLength,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	SrgbColor textColor,
	Transform parent,
	const InterfaceElementEvents* events,
	void* _handle,
	bool isEnabled,
	bool isActive,
	InterfaceElement* uiButton)
{
	assert(ui);
	assert(text);
	assert(textLength > 0);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(scale.x > 0.0f);
	assert(scale.y > 0.0f);
	assert(uiButton);

	assert(!parent || (parent && ui->transformer ==
		getTransformTransformer(parent)));

	UiButtonHandle handle = calloc(1,
		sizeof(UiButtonHandle_T));

	if (!handle)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	handle->ui = ui;
	handle->handle = _handle;
	handle->disabledColor = srgbToLinearColor(DEFAULT_UI_DISABLED_BUTTON_COLOR);
	handle->enabledColor = srgbToLinearColor(DEFAULT_UI_ENABLED_BUTTON_COLOR);
	handle->hoveredColor = srgbToLinearColor(DEFAULT_UI_HOVERED_BUTTON_COLOR);
	handle->pressedColor = srgbToLinearColor(DEFAULT_UI_PRESSED_BUTTON_COLOR);
	handle->isPressed = false;

	Transformer transformer = ui->transformer;

	Transform panelTransform = createTransform(
		transformer,
		zeroVec3F,
		vec3F(scale.x, scale.y, (cmmt_float_t)1.0),
		oneQuat,
		NO_ROTATION_TYPE,
		parent,
		isActive);

	if (!panelTransform)
	{
		onUiButtonDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	GraphicsRender panelRender = createPanelRender(
		ui->panelRenderer,
		panelTransform,
		oneSizeBox3F,
		srgbToLinearColor(DEFAULT_UI_ENABLED_BUTTON_COLOR));

	if (!panelRender)
	{
		destroyTransform(panelTransform);
		onUiButtonDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	handle->panelRender = panelRender;

	Transform textTransform = createTransform(
		transformer,
		vec3F(
			(cmmt_float_t)0.0,
			(cmmt_float_t)0.0,
			(cmmt_float_t)-0.001),
		vec3F(
			DEFAULT_UI_TEXT_HEIGHT,
			DEFAULT_UI_TEXT_HEIGHT,
			(cmmt_float_t)1.0),
		oneQuat,
		NO_ROTATION_TYPE,
		panelTransform,
		true);

	if (!textTransform)
	{
		onUiWindowDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	Text textInstance;

	MpgxResult mpgxResult = createAtlasText(
		ui->fontAtlas,
		text,
		textLength,
		CENTER_ALIGNMENT_TYPE,
		textColor,
		true,
		true,
		&textInstance);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyTransform(textTransform);
		onUiWindowDestroy(handle);
		return mpgxResult;
	}

	GraphicsRender textRender = createTextRender(
		ui->textRenderer,
		textTransform,
		createTextBox3F(alignment,
			getTextSize(textInstance)),
		whiteLinearColor,
		textInstance,
		zeroVec4I);

	if (!textRender)
	{
		destroyText(textInstance);
		destroyTransform(textTransform);
		onUiWindowDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	handle->textRender = textRender;

#ifndef NDEBUG
	const char* name = UI_BUTTON_NAME;
#else
	const char* name = NULL;
#endif

	InterfaceElementEvents elementEvents = events ?
		*events : emptyInterfaceElementEvents;
	handle->onEnable = elementEvents.onEnable;
	handle->onDisable = elementEvents.onDisable;
	handle->onEnter = elementEvents.onEnter;
	handle->onExit = elementEvents.onExit;
	handle->onPress = elementEvents.onPress;
	handle->onRelease = elementEvents.onRelease;
	elementEvents.onEnable = onUiButtonEnable;
	elementEvents.onDisable = onUiButtonDisable;
	elementEvents.onEnter = onUiButtonEnter;
	elementEvents.onExit = onUiButtonExit;
	elementEvents.onPress = onUiButtonPress;
	elementEvents.onRelease = onUiButtonRelease;

	InterfaceElement element = createInterfaceElement(
		ui->interface,
		name,
		panelTransform,
		alignment,
		position,
		oneSizeBox2F,
		isEnabled,
		onUiButtonDestroy,
		&elementEvents,
		handle);

	if (!element)
	{
		onUiButtonDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	*uiButton = element;
	return SUCCESS_MPGX_RESULT;
}
MpgxResult createUiButton8(
	UserInterface ui,
	const char* text,
	size_t textLength,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	SrgbColor textColor,
	Transform parent,
	const InterfaceElementEvents* events,
	void* handle,
	bool isEnabled,
	bool isActive,
	InterfaceElement* uiButton)
{
	assert(ui);
	assert(text);
	assert(textLength > 0);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(scale.x > 0.0f);
	assert(scale.y > 0.0f);
	assert(uiButton);

	assert(!parent || (parent && ui->transformer ==
		getTransformTransformer(parent)));

	uint32_t* text32;
	size_t textLength32;

	MpgxResult mpgxResult = allocateStringUTF32(
		text,
		textLength,
		&text32,
		&textLength32);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	mpgxResult = createUiButton(
		ui,
		text32,
		textLength32,
		alignment,
		position,
		scale,
		textColor,
		parent,
		events,
		handle,
		isEnabled,
		isActive,
		uiButton);

	free(text32);
	return mpgxResult;
}
void* getUiButtonHandle(InterfaceElement button)
{
	assert(button);
	assert(strcmp(getInterfaceElementName(
		button), UI_BUTTON_NAME) == 0);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	return handle->handle;
}
GraphicsRender getUiButtonPanelRender(InterfaceElement button)
{
	assert(button);
	assert(strcmp(getInterfaceElementName(
		button), UI_BUTTON_NAME) == 0);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	return handle->panelRender;
}
GraphicsRender getUiButtonTextRender(InterfaceElement button)
{
	assert(button);
	assert(strcmp(getInterfaceElementName(
		button), UI_BUTTON_NAME) == 0);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	return handle->textRender;
}
OnInterfaceElementEvent getUiButtonOnEnableEvent(InterfaceElement button)
{
	assert(button);
	assert(strcmp(getInterfaceElementName(
		button), UI_BUTTON_NAME) == 0);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	return handle->onEnable;
}
OnInterfaceElementEvent getUiButtonOnDisableEvent(InterfaceElement button)
{
	assert(button);
	assert(strcmp(getInterfaceElementName(
		button), UI_BUTTON_NAME) == 0);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	return handle->onDisable;
}
OnInterfaceElementEvent getUiButtonOnEnterEvent(InterfaceElement button)
{
	assert(button);
	assert(strcmp(getInterfaceElementName(
		button), UI_BUTTON_NAME) == 0);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	return handle->onEnter;
}
OnInterfaceElementEvent getUiButtonOnExitEvent(InterfaceElement button)
{
	assert(button);
	assert(strcmp(getInterfaceElementName(
		button), UI_BUTTON_NAME) == 0);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	return handle->onExit;
}
OnInterfaceElementEvent getUiButtonOnPressEvent(InterfaceElement button)
{
	assert(button);
	assert(strcmp(getInterfaceElementName(
		button), UI_BUTTON_NAME) == 0);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	return handle->onPress;
}
OnInterfaceElementEvent getUiButtonOnReleaseEvent(InterfaceElement button)
{
	assert(button);
	assert(strcmp(getInterfaceElementName(
		button), UI_BUTTON_NAME) == 0);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	return handle->onRelease;
}

LinearColor getUiButtonEnabledColor(
	InterfaceElement button)
{
	assert(button);
	assert(strcmp(getInterfaceElementName(
		button), UI_BUTTON_NAME) == 0);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	return handle->enabledColor;
}
void setUiButtonEnabledColor(
	InterfaceElement button,
	LinearColor color)
{
	assert(button);
	assert(strcmp(getInterfaceElementName(
		button), UI_BUTTON_NAME) == 0);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	handle->enabledColor = color;
}

LinearColor getUiButtonDisabledColor(
	InterfaceElement button)
{
	assert(button);
	assert(strcmp(getInterfaceElementName(
		button), UI_BUTTON_NAME) == 0);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	return handle->disabledColor;
}
void setUiButtonDisabledColor(
	InterfaceElement button,
	LinearColor color)
{
	assert(button);
	assert(strcmp(getInterfaceElementName(
		button), UI_BUTTON_NAME) == 0);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	handle->disabledColor = color;
}

LinearColor getUiButtonHoveredColor(
	InterfaceElement button)
{
	assert(button);
	assert(strcmp(getInterfaceElementName(
		button), UI_BUTTON_NAME) == 0);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	return handle->hoveredColor;
}
void setUiButtonHoveredColor(
	InterfaceElement button,
	LinearColor color)
{
	assert(button);
	assert(strcmp(getInterfaceElementName(
		button), UI_BUTTON_NAME) == 0);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	handle->hoveredColor = color;
}

LinearColor getUiButtonPressedColor(
	InterfaceElement button)
{
	assert(button);
	assert(strcmp(getInterfaceElementName(
		button), UI_BUTTON_NAME) == 0);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	return handle->pressedColor;
}
void setUiButtonPressedColor(
	InterfaceElement button,
	LinearColor color)
{
	assert(button);
	assert(strcmp(getInterfaceElementName(
		button), UI_BUTTON_NAME) == 0);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	handle->pressedColor = color;
}

static void onUiInputFieldEnable(InterfaceElement element)
{
	assert(element);
	UiInputFieldHandle handle = (UiInputFieldHandle)
		getInterfaceElementHandle(element);
	setPanelRenderColor(
		handle->panelRender,
		handle->enabledColor);
	if (handle->onEnable)
		handle->onEnable(element);
}
static void onUiInputFieldDisable(InterfaceElement element)
{
	assert(element);
	UiInputFieldHandle handle = (UiInputFieldHandle)
		getInterfaceElementHandle(element);
	setPanelRenderColor(
		handle->panelRender,
		handle->disabledColor);
	if (handle->onDisable)
		handle->onDisable(element);
}
static void onUiInputFieldEnter(InterfaceElement element)
{
	assert(element);
	UiInputFieldHandle handle = (UiInputFieldHandle)
		getInterfaceElementHandle(element);
	setWindowCursorType(
		handle->ui->window,
		IBEAM_CURSOR_TYPE);
	if (handle->onEnter)
		handle->onEnter(element);
}
static void onUiInputFieldExit(InterfaceElement element)
{
	assert(element);
	UiInputFieldHandle handle = (UiInputFieldHandle)
		getInterfaceElementHandle(element);
	setWindowCursorType(
		handle->ui->window,
		DEFAULT_CURSOR_TYPE);
	if (handle->onExit)
		handle->onExit(element);
}
static void onUiInputFieldPress(InterfaceElement element)
{
	assert(element);
	UiInputFieldHandle handle = (UiInputFieldHandle)
		getInterfaceElementHandle(element);
	setPanelRenderColor(
		handle->focusRender,
		handle->focusedColor);

	UserInterface ui = handle->ui;
	Text text = getTextRenderText(handle->textRender);
	Transform textTransform = getGraphicsRenderTransform(
		handle->placeholderRender);
	updateUiCursor(ui, textTransform, text);

	ui->blinkDelay = getWindowUpdateTime(ui->window) + 0.5f;
	ui->focusedInputField = element;

	if (handle->onPress)
		handle->onPress(element);
}
static void onUiInputFieldDestroy(void* _handle)
{
	assert(_handle);
	UiInputFieldHandle handle = (UiInputFieldHandle)_handle;

	Transform transform;
	GraphicsRender render = handle->placeholderRender;

	if (render)
	{
		Text text = getTextRenderText(render);
		transform = getGraphicsRenderTransform(render);
		destroyGraphicsRender(render);
		destroyText(text);
		destroyTransform(transform);
	}

	render = handle->textRender;

	if (render)
	{
		Text text = getTextRenderText(render);
		transform = getGraphicsRenderTransform(render);
		destroyGraphicsRender(render);
		destroyText(text);
		destroyTransform(transform);
	}

	render = handle->focusRender;

	if (render)
	{
		transform = getGraphicsRenderTransform(render);
		destroyGraphicsRender(render);
		destroyTransform(transform);
	}

	render = handle->panelRender;

	if (render)
	{
		transform = getGraphicsRenderTransform(render);
		destroyGraphicsRender(render);
		destroyTransform(transform);
	}

	free(handle);
}
MpgxResult createUiInputField(
	UserInterface ui,
	const uint32_t* placeholder,
	size_t placeholderLength,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	SrgbColor placeholderColor,
	SrgbColor textColor,
	size_t maxLength,
	Transform parent,
	const InterfaceElementEvents* events,
	void* _handle,
	bool isEnabled,
	bool isActive,
	InterfaceElement* uiInputField)
{
	assert(ui);
	assert(placeholder);
	assert(placeholderLength > 0);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(scale.x > 0.0f);
	assert(scale.y > 0.0f);
	assert(maxLength > 0);
	assert(uiInputField);

	assert(!parent || (parent && ui->transformer ==
		getTransformTransformer(parent)));

	UiInputFieldHandle handle = calloc(1,
		sizeof(UiInputFieldHandle_T));

	if (!handle)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	handle->ui = ui;
	handle->handle = _handle;
	handle->disabledColor = srgbToLinearColor(DEFAULT_UI_DISABLED_INPUT_COLOR);
	handle->enabledColor = srgbToLinearColor(DEFAULT_UI_ENABLED_INPUT_COLOR);
	handle->focusedColor = srgbToLinearColor(DEFAULT_UI_FOCUSED_INPUT_COLOR);
	handle->maxLength = maxLength;

	Transformer transformer = ui->transformer;
	GraphicsRenderer panelRenderer = ui->panelRenderer;
	FontAtlas fontAtlas = ui->fontAtlas;

	Transform panelTransform = createTransform(
		transformer,
		zeroVec3F,
		vec3F(scale.x, scale.y, (cmmt_float_t)1.0),
		oneQuat,
		NO_ROTATION_TYPE,
		parent,
		isActive);

	if (!panelTransform)
	{
		onUiInputFieldDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	GraphicsRender panelRender = createPanelRender(
		panelRenderer,
		panelTransform,
		oneSizeBox3F,
		srgbToLinearColor(DEFAULT_UI_INPUT_PANEL_COLOR));

	if (!panelRender)
	{
		destroyTransform(panelTransform);
		onUiInputFieldDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	handle->panelRender = panelRender;

	Transform focusTransform = createTransform(
		transformer,
		vec3F(
			(cmmt_float_t)0.0,
			(cmmt_float_t)0.0,
			(cmmt_float_t)0.001),
		vec3F(
			scale.x + (cmmt_float_t)4.0,
			scale.y + (cmmt_float_t)4.0,
			(cmmt_float_t)1.0),
		oneQuat,
		NO_ROTATION_TYPE,
		panelTransform,
		true);

	if (!focusTransform)
	{
		onUiInputFieldDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	GraphicsRender focusRender = createPanelRender(
		panelRenderer,
		focusTransform,
		oneSizeBox3F,
		srgbToLinearColor(DEFAULT_UI_ENABLED_BUTTON_COLOR));

	if (!focusRender)
	{
		destroyTransform(focusTransform);
		onUiInputFieldDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	handle->focusRender = focusRender;

	cmmt_float_t textPosition =
		(scale.x * (cmmt_float_t)-0.5) +
		(DEFAULT_UI_TEXT_HEIGHT * (cmmt_float_t)0.5);

	Transform textTransform = createTransform(
		transformer,
		vec3F(
			textPosition,
			(cmmt_float_t)0.0,
			(cmmt_float_t)-0.001),
		vec3F(
			DEFAULT_UI_TEXT_HEIGHT,
			DEFAULT_UI_TEXT_HEIGHT,
			(cmmt_float_t)1.0),
		oneQuat,
		NO_ROTATION_TYPE,
		panelTransform,
		false);

	if (!textTransform)
	{
		onUiInputFieldDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	Text textInstance;

	const uint32_t text[] = { '-', };

	MpgxResult mpgxResult = createAtlasText(
		fontAtlas,
		text,
		sizeof(text) / sizeof(uint32_t),
		CENTER_ALIGNMENT_TYPE,
		textColor,
		false,
		false,
		&textInstance);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyTransform(textTransform);
		onUiInputFieldDestroy(handle);
		return mpgxResult;
	}

	removeTextChar(textInstance, 0);

	GraphicsRender textRender = createTextRender(
		ui->textRenderer,
		textTransform,
		createTextBox3F(
			alignment,
			getTextSize(textInstance)),
		whiteLinearColor,
		textInstance,
		zeroVec4I);

	if (!textRender)
	{
		destroyText(textInstance);
		destroyTransform(textTransform);
		onUiInputFieldDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	handle->textRender = textRender;

	Transform placeholderTransform = createTransform(
		transformer,
		vec3F(
			textPosition,
			(cmmt_float_t)0.0,
			(cmmt_float_t)-0.001),
		vec3F(
			DEFAULT_UI_TEXT_HEIGHT,
			DEFAULT_UI_TEXT_HEIGHT,
			(cmmt_float_t)1.0),
		oneQuat,
		NO_ROTATION_TYPE,
		panelTransform,
		true);

	if (!placeholderTransform)
	{
		onUiInputFieldDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	Text placeholderInstance;

	mpgxResult = createAtlasText(
		fontAtlas,
		placeholder,
		placeholderLength,
		LEFT_ALIGNMENT_TYPE,
		placeholderColor,
		true,
		true,
		&placeholderInstance);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyTransform(placeholderTransform);
		onUiInputFieldDestroy(handle);
		return mpgxResult;
	}

	GraphicsRender placeholderRender = createTextRender(
		ui->textRenderer,
		placeholderTransform,
		createTextBox3F(
			alignment,
			getTextSize(textInstance)),
		whiteLinearColor,
		placeholderInstance,
		zeroVec4I);

	if (!placeholderRender)
	{
		destroyText(placeholderInstance);
		destroyTransform(placeholderTransform);
		onUiInputFieldDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	handle->placeholderRender = placeholderRender;

#ifndef NDEBUG
	const char* name = UI_INPUT_FIELD_NAME;
#else
	const char* name = NULL;
#endif

	InterfaceElementEvents elementEvents = events ?
		*events : emptyInterfaceElementEvents;
	handle->onEnable = elementEvents.onEnable;
	handle->onDisable = elementEvents.onDisable;
	handle->onEnter = elementEvents.onEnter;
	handle->onExit = elementEvents.onExit;
	handle->onPress = elementEvents.onPress;
	elementEvents.onEnable = onUiInputFieldEnable;
	elementEvents.onDisable = onUiInputFieldDisable;
	elementEvents.onEnter = onUiInputFieldEnter;
	elementEvents.onExit = onUiInputFieldExit;
	elementEvents.onPress = onUiInputFieldPress;

	InterfaceElement element = createInterfaceElement(
		ui->interface,
		name,
		panelTransform,
		alignment,
		position,
		oneSizeBox2F,
		isEnabled,
		onUiInputFieldDestroy,
		&elementEvents,
		handle);

	if (!element)
	{
		onUiInputFieldDestroy(handle);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	*uiInputField = element;
	return SUCCESS_MPGX_RESULT;
}

// TODO: possibly abort on bad text operations
