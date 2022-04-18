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
#include "openssl/crypto.h"

#include <string.h>

#if _WIN32
#undef interface
#endif

// TODO: custom delays and cursor color
#define ACTION_START_DELAY 0.24
#define ACTION_PRESS_DELAY 0.08

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
	size_t cursorIndex;
	bool isMousePressed;
	bool isButtonPressed;
};

typedef struct UiBaseHandle_T
{
	UiType type;
} UiBaseHandle_T;
typedef struct UiPanelHandle_T
{
	UiType type;
	void* handle;
	GraphicsRender render;
} UiPanelHandle_T;
typedef struct UiLabelHandle_T
{
	UiType type;
	void* handle;
	GraphicsRender render;
} UiLabelHandle_T;
typedef struct UiWindowHandle_T
{
	UiType type;
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
	UiType type;
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
	UiType type;
	UserInterface ui;
	void* handle;
	OnInterfaceElementEvent onUpdate;
	OnInterfaceElementEvent onEnable;
	OnInterfaceElementEvent onDisable;
	OnInterfaceElementEvent onEnter;
	OnInterfaceElementEvent onExit;
	OnInterfaceElementEvent onPress;
	OnInterfaceElementEvent onChange;
	OnInterfaceElementEvent onDefocus;
	LinearColor disabledColor;
	LinearColor enabledColor;
	LinearColor focusedColor;
	size_t maxLength;
	GraphicsRender panelRender;
	GraphicsRender focusRender;
	GraphicsRender textRender;
	GraphicsRender placeholderRender;
	uint32_t mask;
} UiInputFieldHandle_T;

typedef UiBaseHandle_T* UiBaseHandle;
typedef UiPanelHandle_T* UiPanelHandle;
typedef UiLabelHandle_T* UiLabelHandle;
typedef UiWindowHandle_T* UiWindowHandle;
typedef UiButtonHandle_T* UiButtonHandle;
typedef UiInputFieldHandle_T* UiInputFieldHandle;

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

	ThreadPool threadPool =
		getTransformerThreadPool(transformer);

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
GraphicsRenderer getUserInterfacePanelRenderer(UserInterface ui)
{
	assert(ui);
	return ui->panelRenderer;
}
GraphicsRenderer getUserInterfaceTextRenderer(UserInterface ui)
{
	assert(ui);
	return ui->textRenderer;
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

inline static void bakeUiInputFieldText(
	Text text,
	uint32_t mask,
	Logger logger)
{
	assert(text);

	MpgxResult mpgxResult = OUT_OF_HOST_MEMORY_MPGX_RESULT;
	size_t length = getTextLength(text);

	if (length > 0 && mask != 0) // TODO: use better approach
	{
		const uint32_t* string = getTextString(text);
		uint32_t* textString = malloc(length * sizeof(uint32_t));
		uint32_t* maskString = malloc(length * sizeof(uint32_t));

		if (textString && maskString)
		{
			memcpy(textString, string,
				length * sizeof(uint32_t));

			for (size_t i = 0; i < length; ++i)
				maskString[i] = mask;

			bool result = setTextString(text, maskString, length);

			if (result)
			{
				mpgxResult = bakeText(text);
				setTextString(text, textString, length);
			}

			OPENSSL_cleanse(textString, length * sizeof(uint32_t));
		}

		free(textString);
		free(maskString);
	}
	else
	{
		mpgxResult = bakeText(text);
	}

	if (mpgxResult != SUCCESS_MPGX_RESULT && logger)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to bake text. (error: %s)",
			mpgxResultToString(mpgxResult));
	}
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

	setTransformParent(cursorTransform, textTransform);
	setTransformPosition(cursorTransform, vec3F(
		cursorOffset.x,
		cursorOffset.y,
		(cmmt_float_t)0.0));
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
			defocusUserInterface(ui);
			ui->isMousePressed = true;
		}
	}
	else
	{
		ui->isMousePressed = false;
	}

#if __linux__ || _WIN32
	KeyboardKey leftSuperKey = LEFT_CONTROL_KEYBOARD_KEY;
	KeyboardKey rightSuperKey = RIGHT_CONTROL_KEYBOARD_KEY;
#elif __APPLE__
	KeyboardKey leftSuperKey = LEFT_SUPER_KEYBOARD_KEY;
	KeyboardKey rightSuperKey = RIGHT_SUPER_KEYBOARD_KEY;
#else
#error Unknown operating system
#endif

	if (ui->focusedInputField)
	{
		InterfaceElement focusedInputField = ui->focusedInputField;
		UiInputFieldHandle handle = getInterfaceElementHandle(focusedInputField);
		Text text = getTextRenderText(handle->textRender);
		double updateTime = getWindowUpdateTime(window);

		bool isTextChanged = false, isCursorChanged = false;

		if (getWindowKeyboardKey(window, BACKSPACE_KEYBOARD_KEY))
		{
			if (getTextLength(text) > 0 && ui->cursorIndex > 0 &&
				(!ui->isButtonPressed || ui->buttonDelay < updateTime))
			{
				ui->cursorIndex--;
				removeTextChar(text, ui->cursorIndex);
				ui->buttonDelay = ui->isButtonPressed ?
					updateTime + ACTION_PRESS_DELAY : updateTime + ACTION_START_DELAY;
				ui->isButtonPressed = isTextChanged = isCursorChanged = true;
			}
		}
		else if (getWindowKeyboardKey(window, DELETE_KEYBOARD_KEY))
		{
			if (getTextLength(text) > 0 && ui->cursorIndex < getTextLength(text) &&
				(!ui->isButtonPressed || ui->buttonDelay < updateTime))
			{
				removeTextChar(text, ui->cursorIndex);
				ui->buttonDelay = ui->isButtonPressed ?
					updateTime + ACTION_PRESS_DELAY : updateTime + ACTION_START_DELAY;
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
					updateTime + ACTION_PRESS_DELAY : updateTime + ACTION_START_DELAY;
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
					updateTime + ACTION_PRESS_DELAY : updateTime + ACTION_START_DELAY;
				ui->isButtonPressed = isCursorChanged = true;
			}
		}
		else if (getWindowKeyboardKey(window, V_KEYBOARD_KEY) &&
			(getWindowKeyboardKey(window, leftSuperKey) ||
			getWindowKeyboardKey(window, rightSuperKey)))
		{
			if (!ui->isButtonPressed || ui->buttonDelay < updateTime)
			{
				const char* clipboard = getWindowClipboard(window);
				size_t length = strlen(clipboard);

				if (length > 0)
				{
					uint32_t* clipboard32;
					size_t length32;

					MpgxResult mpgxResult = allocateStringUTF32(
						clipboard,
						length,
						&clipboard32,
						&length32);

					if (mpgxResult == SUCCESS_MPGX_RESULT)
					{
						if (getTextLength(text) + length32 > handle->maxLength)
							length32 = handle->maxLength - getTextLength(text);

						if (length32 > 0)
						{
							bool result = appendTextString32(text,
								clipboard32,
								length32,
								ui->cursorIndex);

							if (result)
							{
								ui->cursorIndex += length32;
								isTextChanged = isCursorChanged = true;
							}
						}

						free(clipboard32);
					}
				}

				ui->buttonDelay = ui->isButtonPressed ?
					updateTime + ACTION_PRESS_DELAY : updateTime + ACTION_START_DELAY;
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
			if (getTextLength(text) > 0)
			{
				bakeUiInputFieldText(text,
					handle->mask,
					getFontAtlasLogger(ui->fontAtlas));
				setTransformActive(getGraphicsRenderTransform(
					handle->textRender), true);
				setTransformActive(getGraphicsRenderTransform(
					handle->placeholderRender), false);
			}
			else
			{
				setTransformActive(getGraphicsRenderTransform(
					handle->textRender), false);
				setTransformActive(getGraphicsRenderTransform(
					handle->placeholderRender), true);
			}

			if (handle->onChange)
				handle->onChange(focusedInputField);
		}
		if (isCursorChanged)
		{
			Transform textTransform = getGraphicsRenderTransform(
				getTextLength(text) > 0 ?
				handle->textRender : handle->placeholderRender);
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

static void onUiElementScissor(
	InterfaceElement element,
	void* handle)
{
	assert(element);
	assert(handle);

	UserInterface ui = (UserInterface)handle;

	// TODO: add enumerateInterfaceAsync
}
GraphicsRendererResult drawUserInterface(UserInterface ui)
{
	assert(ui);

	enumerateInterfaceElements(
		ui->interface,
		onUiElementScissor,
		ui);

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
void defocusUserInterface(UserInterface ui)
{
	assert(ui);

	if (ui->focusedInputField)
	{
		InterfaceElement focusedInputField = ui->focusedInputField;
		UiInputFieldHandle handle = getInterfaceElementHandle(focusedInputField);
		Transform cursorTransform = getGraphicsRenderTransform(ui->cursorRender);
		setPanelRenderColor(handle->focusRender, handle->enabledColor);
		setTransformParent(cursorTransform, NULL);
		setTransformActive(cursorTransform, false);
		ui->focusedInputField = NULL;

		if (handle->onDefocus)
			handle->onDefocus(focusedInputField);
	}
}

UiType getUiType(InterfaceElement element)
{
	assert(element);
	UiBaseHandle handle = getInterfaceElementHandle(element);
	return handle->type;
}

static void onUiPanelDestroy(void* _handle)
{
	assert(_handle);
	UiPanelHandle handle = (UiPanelHandle)_handle;
	assert(handle->type == PANEL_UI_TYPE);

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

	handle->type = PANEL_UI_TYPE;
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

	InterfaceElement element = createInterfaceElement(
		ui->interface,
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
	UiPanelHandle handle =
		getInterfaceElementHandle(panel);
	assert(handle->type == PANEL_UI_TYPE);
	return handle->handle;
}
GraphicsRender getUiPanelRender(InterfaceElement panel)
{
	assert(panel);
	UiPanelHandle handle =
		getInterfaceElementHandle(panel);
	assert(handle->type == PANEL_UI_TYPE);
	return handle->render;
}

static void onUiLabelDestroy(void* _handle)
{
	assert(_handle);
	UiLabelHandle handle = (UiLabelHandle)_handle;
	assert(handle->type == LABEL_UI_TYPE);

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
inline static MpgxResult internalCreateUiLabel(
	UserInterface ui,
	const void* string,
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
	void* _handle,
	bool isActive,
	InterfaceElement* uiLabel,
	bool isUTF8)
{
	assert(ui);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(scale > 0.0f);
	assert(uiLabel);

	assert(stringLength == 0 ||
		(stringLength > 0 && string));
	assert(!parent || (parent && ui->transformer ==
		getTransformTransformer(parent)));

	UiLabelHandle handle = calloc(1,
		sizeof(UiButtonHandle_T));

	if (!handle)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	handle->type = LABEL_UI_TYPE;
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
	MpgxResult mpgxResult;

	if (isUniversal)
	{
		uint32_t fontSize = (uint32_t)(scale * 2 * getPlatformScale(
			getWindowFramebuffer(ui->window)));
		FontAtlas fontAtlas = ui->fontAtlas;

		if (fontSize % 2 != 0)
			fontSize += 1;

		if (isUTF8)
		{
			mpgxResult = createFontText8(
				getFontAtlasPipeline(fontAtlas),
				getFontAtlasRegularFonts(fontAtlas),
				getFontAtlasBoldFonts(fontAtlas),
				getFontAtlasItalicFonts(fontAtlas),
				getFontAtlasBoldItalicFonts(fontAtlas),
				getFontAtlasFontCount(fontAtlas),
				fontSize,
				string,
				stringLength,
				alignment,
				color,
				isBold,
				isItalic,
				useTags,
				isConstant,
				getFontAtlasLogger(fontAtlas),
				&text);
		}
		else
		{
			mpgxResult = createFontText(
				getFontAtlasPipeline(fontAtlas),
				getFontAtlasRegularFonts(fontAtlas),
				getFontAtlasBoldFonts(fontAtlas),
				getFontAtlasItalicFonts(fontAtlas),
				getFontAtlasBoldItalicFonts(fontAtlas),
				getFontAtlasFontCount(fontAtlas),
				fontSize,
				string,
				stringLength,
				alignment,
				color,
				isBold,
				isItalic,
				useTags,
				isConstant,
				getFontAtlasLogger(fontAtlas),
				&text);
		}
	}
	else
	{
		if (isUTF8)
		{
			mpgxResult = createAtlasText8(
				ui->fontAtlas,
				string,
				stringLength,
				alignment,
				color,
				isBold,
				isItalic,
				useTags,
				isConstant,
				&text);
		}
		else
		{
			mpgxResult = createAtlasText(
				ui->fontAtlas,
				string,
				stringLength,
				alignment,
				color,
				isBold,
				isItalic,
				useTags,
				isConstant,
				&text);
		}
	}

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

	InterfaceElement element = createInterfaceElement(
		ui->interface,
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
	InterfaceElement* uiLabel)
{
	assert(ui);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(scale > 0.0f);
	assert(uiLabel);

	assert(stringLength == 0 ||
		(stringLength > 0 && string));
	assert(!parent || (parent && ui->transformer ==
		getTransformTransformer(parent)));

	return internalCreateUiLabel(
		ui,
		string,
		stringLength,
		alignment,
		position,
		scale,
		color,
		isBold,
		isItalic,
		useTags,
		isConstant,
		isUniversal,
		parent,
		events,
		handle,
		isActive,
		uiLabel,
		false);
}
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
	InterfaceElement* uiLabel)
{
	assert(ui);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(scale > 0.0f);
	assert(uiLabel);

	assert(stringLength == 0 ||
		(stringLength > 0 && string));
	assert(!parent || (parent && ui->transformer ==
		getTransformTransformer(parent)));

	return internalCreateUiLabel(
		ui,
		string,
		stringLength,
		alignment,
		position,
		scale,
		color,
		isBold,
		isItalic,
		useTags,
		isConstant,
		isUniversal,
		parent,
		events,
		handle,
		isActive,
		uiLabel,
		true);
}

void* getUiLabelHandle(InterfaceElement label)
{
	assert(label);
	UiLabelHandle handle =
		getInterfaceElementHandle(label);
	assert(handle->type == LABEL_UI_TYPE);
	return handle->handle;
}
GraphicsRender getUiLabelRender(InterfaceElement label)
{
	assert(label);
	UiLabelHandle handle =
		getInterfaceElementHandle(label);
	assert(handle->type == LABEL_UI_TYPE);
	return handle->render;
}

static void onUiWindowPress(InterfaceElement element)
{
	assert(element);
	UiWindowHandle handle = (UiWindowHandle)
		getInterfaceElementHandle(element);
	assert(handle->type == WINDOW_UI_TYPE);

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
	assert(handle->type == WINDOW_UI_TYPE);

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
	assert(handle->type == WINDOW_UI_TYPE);

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
inline static MpgxResult internalCreateUiWindow(
	UserInterface ui,
	const void* title,
	size_t titleLength,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	Transform parent,
	const InterfaceElementEvents* events,
	void* _handle,
	bool isActive,
	InterfaceElement* uiWindow,
	bool isUTF8)
{
	assert(ui);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(scale.x > 0.0f);
	assert(scale.y > 0.0f);
	assert(uiWindow);

	assert(titleLength == 0 ||
		(titleLength > 0 && title));
	assert(!parent || (parent && ui->transformer ==
		getTransformTransformer(parent)));

	UiWindowHandle handle = calloc(1,
		sizeof(UiWindowHandle_T));

	if (!handle)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	handle->type = WINDOW_UI_TYPE;
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
	MpgxResult mpgxResult;

	if (isUTF8)
	{
		mpgxResult = createAtlasText8(
			ui->fontAtlas,
			title,
			titleLength,
			CENTER_ALIGNMENT_TYPE,
			DEFAULT_UI_TEXT_COLOR,
			true,
			false,
			true,
			true,
			&text);
	}
	else
	{
		mpgxResult = createAtlasText(
			ui->fontAtlas,
			title,
			titleLength,
			CENTER_ALIGNMENT_TYPE,
			DEFAULT_UI_TEXT_COLOR,
			true,
			false,
			true,
			true,
			&text);
	}

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

	InterfaceElementEvents elementEvents = events ?
		*events : emptyInterfaceElementEvents;
	handle->onUpdate = elementEvents.onUpdate;
	handle->onPress = elementEvents.onPress;
	elementEvents.onUpdate = onUiWindowUpdate;
	elementEvents.onPress = onUiWindowPress;

	InterfaceElement element = createInterfaceElement(
		ui->interface,
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
	InterfaceElement* uiWindow)
{
	assert(ui);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(scale.x > 0.0f);
	assert(scale.y > 0.0f);
	assert(uiWindow);

	assert(titleLength == 0 ||
		(titleLength > 0 && title));
	assert(!parent || (parent && ui->transformer ==
		getTransformTransformer(parent)));

	return internalCreateUiWindow(
		ui,
		title,
		titleLength,
		alignment,
		position,
		scale,
		parent,
		events,
		handle,
		isActive,
		uiWindow,
		false);
}
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
	InterfaceElement* uiWindow)
{
	assert(ui);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(scale.x > 0.0f);
	assert(scale.y > 0.0f);
	assert(uiWindow);

	assert(titleLength == 0 ||
		(titleLength > 0 && title));
	assert(!parent || (parent && ui->transformer ==
		getTransformTransformer(parent)));

	return internalCreateUiWindow(
		ui,
		title,
		titleLength,
		alignment,
		position,
		scale,
		parent,
		events,
		handle,
		isActive,
		uiWindow,
		true);
}

void* getUiWindowHandle(InterfaceElement window)
{
	assert(window);
	UiWindowHandle handle =
		getInterfaceElementHandle(window);
	assert(handle->type == WINDOW_UI_TYPE);
	return handle->handle;
}
GraphicsRender getUiWindowPanelRender(InterfaceElement window)
{
	assert(window);
	UiWindowHandle handle =
		getInterfaceElementHandle(window);
	assert(handle->type == WINDOW_UI_TYPE);
	return handle->panelRender;
}
GraphicsRender getUiWindowBarRender(InterfaceElement window)
{
	assert(window);
	UiWindowHandle handle =
		getInterfaceElementHandle(window);
	assert(handle->type == WINDOW_UI_TYPE);
	return handle->barRender;
}
GraphicsRender getUiWindowTitleRender(InterfaceElement window)
{
	assert(window);
	UiWindowHandle handle =
		getInterfaceElementHandle(window);
	assert(handle->type == WINDOW_UI_TYPE);
	return handle->titleRender;
}
OnInterfaceElementEvent getUiWindowOnUpdateEvent(InterfaceElement window)
{
	assert(window);
	UiWindowHandle handle =
		getInterfaceElementHandle(window);
	assert(handle->type == WINDOW_UI_TYPE);
	return handle->onUpdate;
}
OnInterfaceElementEvent getUiWindowOnPressEvent(InterfaceElement window)
{
	assert(window);
	UiWindowHandle handle =
		getInterfaceElementHandle(window);
	assert(handle->type == WINDOW_UI_TYPE);
	return handle->onPress;
}

static void onUiButtonEnable(InterfaceElement element)
{
	assert(element);
	UiButtonHandle handle = (UiButtonHandle)
		getInterfaceElementHandle(element);
	assert(handle->type == BUTTON_UI_TYPE);
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
	assert(handle->type == BUTTON_UI_TYPE);
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
	assert(handle->type == BUTTON_UI_TYPE);
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
	assert(handle->type == BUTTON_UI_TYPE);
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
	assert(handle->type == BUTTON_UI_TYPE);
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
	assert(handle->type == BUTTON_UI_TYPE);
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
	assert(handle->type == BUTTON_UI_TYPE);

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
inline static MpgxResult internalCreateUiButton(
	UserInterface ui,
	const void* text,
	size_t textLength,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	Transform parent,
	const InterfaceElementEvents* events,
	void* _handle,
	bool isActive,
	InterfaceElement* uiButton,
	bool isUTF8)
{
	assert(ui);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(scale.x > 0.0f);
	assert(scale.y > 0.0f);
	assert(uiButton);

	assert(textLength == 0 ||
		(textLength > 0 && text));
	assert(!parent || (parent && ui->transformer ==
		getTransformTransformer(parent)));

	UiButtonHandle handle = calloc(1,
		sizeof(UiButtonHandle_T));

	if (!handle)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	handle->type = BUTTON_UI_TYPE;
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
	MpgxResult mpgxResult;

	if (isUTF8)
	{
		mpgxResult = createAtlasText8(
			ui->fontAtlas,
			text,
			textLength,
			CENTER_ALIGNMENT_TYPE,
			DEFAULT_UI_TEXT_COLOR,
			true,
			false,
			true,
			true,
			&textInstance);
	}
	else
	{
		mpgxResult = createAtlasText(
			ui->fontAtlas,
			text,
			textLength,
			CENTER_ALIGNMENT_TYPE,
			DEFAULT_UI_TEXT_COLOR,
			true,
			false,
			true,
			true,
			&textInstance);
	}

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
		panelTransform,
		alignment,
		position,
		oneSizeBox2F,
		true,
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
MpgxResult createUiButton(
	UserInterface ui,
	const uint32_t* text,
	size_t textLength,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	Transform parent,
	const InterfaceElementEvents* events,
	void* handle,
	bool isActive,
	InterfaceElement* uiButton)
{
	assert(ui);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(scale.x > 0.0f);
	assert(scale.y > 0.0f);
	assert(uiButton);

	assert(textLength == 0 ||
		(textLength > 0 && text));
	assert(!parent || (parent && ui->transformer ==
		getTransformTransformer(parent)));

	return internalCreateUiButton(
		ui,
		text,
		textLength,
		alignment,
		position,
		scale,
		parent,
		events,
		handle,
		isActive,
		uiButton,
		false);
}
MpgxResult createUiButton8(
	UserInterface ui,
	const char* text,
	size_t textLength,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	Transform parent,
	const InterfaceElementEvents* events,
	void* handle,
	bool isActive,
	InterfaceElement* uiButton)
{
	assert(ui);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(scale.x > 0.0f);
	assert(scale.y > 0.0f);
	assert(uiButton);

	assert(textLength == 0 ||
		(textLength > 0 && text));
	assert(!parent || (parent && ui->transformer ==
		getTransformTransformer(parent)));

	return internalCreateUiButton(
		ui,
		text,
		textLength,
		alignment,
		position,
		scale,
		parent,
		events,
		handle,
		isActive,
		uiButton,
		true);
}
void* getUiButtonHandle(InterfaceElement button)
{
	assert(button);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	assert(handle->type == BUTTON_UI_TYPE);
	return handle->handle;
}
GraphicsRender getUiButtonPanelRender(InterfaceElement button)
{
	assert(button);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	assert(handle->type == BUTTON_UI_TYPE);
	return handle->panelRender;
}
GraphicsRender getUiButtonTextRender(InterfaceElement button)
{
	assert(button);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	assert(handle->type == BUTTON_UI_TYPE);
	return handle->textRender;
}
OnInterfaceElementEvent getUiButtonOnEnableEvent(InterfaceElement button)
{
	assert(button);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	assert(handle->type == BUTTON_UI_TYPE);
	return handle->onEnable;
}
OnInterfaceElementEvent getUiButtonOnDisableEvent(InterfaceElement button)
{
	assert(button);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	assert(handle->type == BUTTON_UI_TYPE);
	return handle->onDisable;
}
OnInterfaceElementEvent getUiButtonOnEnterEvent(InterfaceElement button)
{
	assert(button);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	assert(handle->type == BUTTON_UI_TYPE);
	return handle->onEnter;
}
OnInterfaceElementEvent getUiButtonOnExitEvent(InterfaceElement button)
{
	assert(button);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	assert(handle->type == BUTTON_UI_TYPE);
	return handle->onExit;
}
OnInterfaceElementEvent getUiButtonOnPressEvent(InterfaceElement button)
{
	assert(button);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	assert(handle->type == BUTTON_UI_TYPE);
	return handle->onPress;
}
OnInterfaceElementEvent getUiButtonOnReleaseEvent(InterfaceElement button)
{
	assert(button);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	assert(handle->type == BUTTON_UI_TYPE);
	return handle->onRelease;
}

LinearColor getUiButtonDisabledColor(
	InterfaceElement button)
{
	assert(button);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	assert(handle->type == BUTTON_UI_TYPE);
	return handle->disabledColor;
}
void setUiButtonDisabledColor(
	InterfaceElement button,
	LinearColor color)
{
	assert(button);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	assert(handle->type == BUTTON_UI_TYPE);
	handle->disabledColor = color;
}

LinearColor getUiButtonEnabledColor(
	InterfaceElement button)
{
	assert(button);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	assert(handle->type == BUTTON_UI_TYPE);
	return handle->enabledColor;
}
void setUiButtonEnabledColor(
	InterfaceElement button,
	LinearColor color)
{
	assert(button);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	assert(handle->type == BUTTON_UI_TYPE);
	handle->enabledColor = color;
}

LinearColor getUiButtonHoveredColor(
	InterfaceElement button)
{
	assert(button);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	assert(handle->type == BUTTON_UI_TYPE);
	return handle->hoveredColor;
}
void setUiButtonHoveredColor(
	InterfaceElement button,
	LinearColor color)
{
	assert(button);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	assert(handle->type == BUTTON_UI_TYPE);
	handle->hoveredColor = color;
}

LinearColor getUiButtonPressedColor(
	InterfaceElement button)
{
	assert(button);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	assert(handle->type == BUTTON_UI_TYPE);
	return handle->pressedColor;
}
void setUiButtonPressedColor(
	InterfaceElement button,
	LinearColor color)
{
	assert(button);
	UiButtonHandle handle =
		getInterfaceElementHandle(button);
	assert(handle->type == BUTTON_UI_TYPE);
	handle->pressedColor = color;
}

static void onUiInputFieldUpdate(InterfaceElement element)
{
	assert(element);
	UiInputFieldHandle handle = (UiInputFieldHandle)
		getInterfaceElementHandle(element);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
	UserInterface ui = handle->ui;
	Framebuffer framebuffer = getWindowFramebuffer(ui->window);
	cmmt_float_t platformScale = getPlatformScale(framebuffer);
	Transform panelTransform = getGraphicsRenderTransform(handle->panelRender);
	cmmt_float_t interfaceScale = getInterfaceScale(getUserInterface(ui));
	Vec3F panelPosition = mulValVec3F(getTranslationMat4F(
		getTransformModel(panelTransform)),
		platformScale * interfaceScale);
	Vec3F panelScale = mulValVec3F(
		getTransformScale(panelTransform),
		platformScale * interfaceScale);
	Vec2I framebufferSize = getFramebufferSize(framebuffer);

	Vec4I scissor = vec4I(
		(cmmt_int_t)((cmmt_float_t)framebufferSize.x * (cmmt_float_t)0.5 +
			panelPosition.x - panelScale.x * (cmmt_float_t)0.5),
		(cmmt_int_t)((cmmt_float_t)framebufferSize.y * (cmmt_float_t)0.5 +
			panelPosition.y - panelScale.y * (cmmt_float_t)0.5),
		(cmmt_int_t)panelScale.x,
		(cmmt_int_t)panelScale.y);

	if (scissor.x < 0) scissor.x = 0;
	if (scissor.y < 0) scissor.y = 0;
	if (scissor.z <= 0) scissor.z = 1;
	if (scissor.w <= 0) scissor.w = 1;

	if (scissor.x + scissor.z > framebufferSize.x)
	{
		scissor.x = 0;
		scissor.z = framebufferSize.x;
	}
	if (scissor.y + scissor.w > framebufferSize.y)
	{
		scissor.y = 0;
		scissor.w = framebufferSize.y;
	}

	setTextRenderScissor(handle->textRender, scissor);
	setTextRenderScissor(handle->placeholderRender, scissor);
}
static void onUiInputFieldEnable(InterfaceElement element)
{
	assert(element);
	UiInputFieldHandle handle = (UiInputFieldHandle)
		getInterfaceElementHandle(element);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
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
	assert(handle->type == INPUT_FIELD_UI_TYPE);
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
	assert(handle->type == INPUT_FIELD_UI_TYPE);
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
	assert(handle->type == INPUT_FIELD_UI_TYPE);
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
	assert(handle->type == INPUT_FIELD_UI_TYPE);

	setPanelRenderColor(
		handle->focusRender,
		handle->focusedColor);

	UserInterface ui = handle->ui;
	Window window = ui->window;
	Text text = getTextRenderText(handle->textRender);
	Transform textTransform = getGraphicsRenderTransform(
		getTextLength(text) > 0 ?
		handle->textRender : handle->placeholderRender);
	Vec3F textPosition = getTranslationMat4F(
		getTransformModel(textTransform));
	cmmt_float_t interfaceScale = getInterfaceScale(
		getUserInterface(ui));
	Vec3F textScale = getTransformScale(textTransform);
	Vec2I windowSize = getWindowSize(window);
	Vec2F cursorPosition = getWindowCursorPosition(window);

	Vec2F size = vec2F(
		(cmmt_float_t)windowSize.x / interfaceScale,
		(cmmt_float_t)windowSize.y / interfaceScale);
	Vec2F halfSize = mulValVec2F(size, (cmmt_float_t)0.5);

	cursorPosition = vec2F(
		(cursorPosition.x / interfaceScale) - halfSize.x,
		(size.y - (cursorPosition.y / interfaceScale)) - halfSize.y);
	cursorPosition.x = (cursorPosition.x - textPosition.x) / textScale.x;
	cursorPosition.y = (cursorPosition.y - textPosition.y) / textScale.y;

	size_t index = 0;
	getTextCursorIndex(text, cursorPosition, &index);
	ui->cursorIndex = index;
	updateUiCursor(ui, textTransform, text);

	ui->blinkDelay = getWindowUpdateTime(window) + 0.5f;
	ui->focusedInputField = element;

	if (handle->onPress)
		handle->onPress(element);
}
static void onUiInputFieldDestroy(void* _handle)
{
	assert(_handle);
	UiInputFieldHandle handle = (UiInputFieldHandle)_handle;
	assert(handle->type == INPUT_FIELD_UI_TYPE);

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
inline static MpgxResult internalCreateUiInputField(
	UserInterface ui,
	const void* placeholder,
	size_t placeholderLength,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	size_t maxLength,
	uint32_t mask,
	Transform parent,
	const InterfaceElementEvents* events,
	OnInterfaceElementEvent onChange,
	OnInterfaceElementEvent onDefocus,
	void* _handle,
	bool isActive,
	InterfaceElement* uiInputField,
	bool isUTF8)
{
	assert(ui);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(scale.x > 0.0f);
	assert(scale.y > 0.0f);
	assert(maxLength > 0);
	assert(uiInputField);

	assert(placeholderLength == 0 ||
		(placeholderLength > 0 && placeholder));
	assert(!parent || (parent && ui->transformer ==
		getTransformTransformer(parent)));

	UiInputFieldHandle handle = calloc(1,
		sizeof(UiInputFieldHandle_T));

	if (!handle)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	handle->type = INPUT_FIELD_UI_TYPE;
	handle->ui = ui;
	handle->handle = _handle;
	handle->onChange = onChange;
	handle->onDefocus = onDefocus;
	handle->disabledColor = srgbToLinearColor(DEFAULT_UI_DISABLED_INPUT_COLOR);
	handle->enabledColor = srgbToLinearColor(DEFAULT_UI_ENABLED_INPUT_COLOR);
	handle->focusedColor = srgbToLinearColor(DEFAULT_UI_FOCUSED_INPUT_COLOR);
	handle->maxLength = maxLength;
	handle->mask = mask;

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

	uint32_t fontSize = (uint32_t)(DEFAULT_UI_TEXT_HEIGHT * 2 *
		getPlatformScale(getWindowFramebuffer(ui->window)));
	const uint32_t text[] = { '-', };

	MpgxResult mpgxResult = createFontText(
		getFontAtlasPipeline(fontAtlas),
		getFontAtlasRegularFonts(fontAtlas),
		getFontAtlasBoldFonts(fontAtlas),
		getFontAtlasItalicFonts(fontAtlas),
		getFontAtlasBoldItalicFonts(fontAtlas),
		getFontAtlasFontCount(fontAtlas),
		fontSize,
		text,
		1,
		LEFT_ALIGNMENT_TYPE,
		DEFAULT_UI_TEXT_COLOR,
		false,
		false,
		false,
		false,
		getFontAtlasLogger(fontAtlas),
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

	if (isUTF8)
	{
		mpgxResult = createAtlasText8(
			fontAtlas,
			placeholder,
			placeholderLength,
			LEFT_ALIGNMENT_TYPE,
			DEFAULT_UI_PLACEHOLDER_COLOR,
			false,
			false,
			true,
			true,
			&placeholderInstance);
	}
	else
	{
		mpgxResult = createAtlasText(
			fontAtlas,
			placeholder,
			placeholderLength,
			LEFT_ALIGNMENT_TYPE,
			DEFAULT_UI_PLACEHOLDER_COLOR,
			false,
			false,
			true,
			true,
			&placeholderInstance);
	}

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

	InterfaceElementEvents elementEvents = events ?
		*events : emptyInterfaceElementEvents;
	handle->onUpdate = elementEvents.onUpdate;
	handle->onEnable = elementEvents.onEnable;
	handle->onDisable = elementEvents.onDisable;
	handle->onEnter = elementEvents.onEnter;
	handle->onExit = elementEvents.onExit;
	handle->onPress = elementEvents.onPress;
	elementEvents.onUpdate = onUiInputFieldUpdate;
	elementEvents.onEnable = onUiInputFieldEnable;
	elementEvents.onDisable = onUiInputFieldDisable;
	elementEvents.onEnter = onUiInputFieldEnter;
	elementEvents.onExit = onUiInputFieldExit;
	elementEvents.onPress = onUiInputFieldPress;

	InterfaceElement element = createInterfaceElement(
		ui->interface,
		panelTransform,
		alignment,
		position,
		oneSizeBox2F,
		true,
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
MpgxResult createUiInputField(
	UserInterface ui,
	const uint32_t* placeholder,
	size_t placeholderLength,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	size_t maxLength,
	uint32_t mask,
	Transform parent,
	const InterfaceElementEvents* events,
	OnInterfaceElementEvent onChange,
	OnInterfaceElementEvent onDefocus,
	void* handle,
	bool isActive,
	InterfaceElement* uiInputField)
{
	assert(ui);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(scale.x > 0.0f);
	assert(scale.y > 0.0f);
	assert(maxLength > 0);
	assert(uiInputField);

	assert(placeholderLength == 0 ||
		(placeholderLength > 0 && placeholder));
	assert(!parent || (parent && ui->transformer ==
		getTransformTransformer(parent)));

	return internalCreateUiInputField(
		ui,
		placeholder,
		placeholderLength,
		alignment,
		position,
		scale,
		maxLength,
		mask,
		parent,
		events,
		onChange,
		onDefocus,
		handle,
		isActive,
		uiInputField,
		false);
}
MpgxResult createUiInputField8(
	UserInterface ui,
	const uint32_t* placeholder,
	size_t placeholderLength,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	size_t maxLength,
	uint32_t mask,
	Transform parent,
	const InterfaceElementEvents* events,
	OnInterfaceElementEvent onChange,
	OnInterfaceElementEvent onDefocus,
	void* handle,
	bool isActive,
	InterfaceElement* uiInputField)
{
	assert(ui);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(scale.x > 0.0f);
	assert(scale.y > 0.0f);
	assert(maxLength > 0);
	assert(uiInputField);

	assert(placeholderLength == 0 ||
		(placeholderLength > 0 && placeholder));
	assert(!parent || (parent && ui->transformer ==
		getTransformTransformer(parent)));

	return internalCreateUiInputField(
		ui,
		placeholder,
		placeholderLength,
		alignment,
		position,
		scale,
		maxLength,
		mask,
		parent,
		events,
		onChange,
		onDefocus,
		handle,
		isActive,
		uiInputField,
		true);
}

void* getUiInputFieldHandle(InterfaceElement inputField)
{
	assert(inputField);
	UiInputFieldHandle handle =
		getInterfaceElementHandle(inputField);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
	return handle->handle;
}
GraphicsRender getUiInputFieldPanelRender(InterfaceElement inputField)
{
	assert(inputField);
	UiInputFieldHandle handle =
		getInterfaceElementHandle(inputField);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
	return handle->panelRender;
}
GraphicsRender getUiInputFieldFocusRender(InterfaceElement inputField)
{
	assert(inputField);
	UiInputFieldHandle handle =
		getInterfaceElementHandle(inputField);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
	return handle->focusRender;
}
GraphicsRender getUiInputFieldTextRender(InterfaceElement inputField)
{
	assert(inputField);
	UiInputFieldHandle handle =
		getInterfaceElementHandle(inputField);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
	return handle->textRender;
}
GraphicsRender getUiInputFieldPlaceholderRender(InterfaceElement inputField)
{
	assert(inputField);
	UiInputFieldHandle handle =
		getInterfaceElementHandle(inputField);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
	return handle->placeholderRender;
}
OnInterfaceElementEvent getUiInputFieldOnUpdateEvent(InterfaceElement inputField)
{
	assert(inputField);
	UiInputFieldHandle handle =
		getInterfaceElementHandle(inputField);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
	return handle->onUpdate;
}
OnInterfaceElementEvent getUiInputFieldOnEnableEvent(InterfaceElement inputField)
{
	assert(inputField);
	UiInputFieldHandle handle =
		getInterfaceElementHandle(inputField);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
	return handle->onEnable;
}
OnInterfaceElementEvent getUiInputFieldOnDisableEvent(InterfaceElement inputField)
{
	assert(inputField);
	UiInputFieldHandle handle =
		getInterfaceElementHandle(inputField);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
	return handle->onDisable;
}
OnInterfaceElementEvent getUiInputFieldOnEnterEvent(InterfaceElement inputField)
{
	assert(inputField);
	UiInputFieldHandle handle =
		getInterfaceElementHandle(inputField);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
	return handle->onEnter;
}
OnInterfaceElementEvent getUiInputFieldOnExitEvent(InterfaceElement inputField)
{
	assert(inputField);
	UiInputFieldHandle handle =
		getInterfaceElementHandle(inputField);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
	return handle->onExit;
}
OnInterfaceElementEvent getUiInputFieldOnPressEvent(InterfaceElement inputField)
{
	assert(inputField);
	UiInputFieldHandle handle =
		getInterfaceElementHandle(inputField);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
	return handle->onPress;
}
OnInterfaceElementEvent getUiInputFieldOnChangeEvent(InterfaceElement inputField)
{
	assert(inputField);
	UiInputFieldHandle handle =
		getInterfaceElementHandle(inputField);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
	return handle->onChange;
}
OnInterfaceElementEvent getUiInputFieldOnDefocusEvent(InterfaceElement inputField)
{
	assert(inputField);
	UiInputFieldHandle handle =
		getInterfaceElementHandle(inputField);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
	return handle->onDefocus;
}
size_t getUiInputFieldMaxLength(InterfaceElement inputField)
{
	assert(inputField);
	UiInputFieldHandle handle =
		getInterfaceElementHandle(inputField);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
	return handle->maxLength;
}

LinearColor getUiInputFieldDisabledColor(
	InterfaceElement inputField)
{
	assert(inputField);
	UiInputFieldHandle handle =
		getInterfaceElementHandle(inputField);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
	return handle->disabledColor;
}
void setUiInputFieldDisabledColor(
	InterfaceElement inputField,
	LinearColor color)
{
	assert(inputField);
	UiInputFieldHandle handle =
		getInterfaceElementHandle(inputField);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
	handle->disabledColor = color;
}

LinearColor getUiInputFieldEnabledColor(
	InterfaceElement inputField)
{
	assert(inputField);
	UiInputFieldHandle handle =
		getInterfaceElementHandle(inputField);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
	return handle->enabledColor;
}
void setUiInputFieldEnabledColor(
	InterfaceElement inputField,
	LinearColor color)
{
	assert(inputField);
	UiInputFieldHandle handle =
		getInterfaceElementHandle(inputField);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
	handle->enabledColor = color;
}

LinearColor getUiInputFieldFocusedColor(
	InterfaceElement inputField)
{
	assert(inputField);
	UiInputFieldHandle handle =
		getInterfaceElementHandle(inputField);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
	return handle->focusedColor;
}
void setUiInputFieldFocusedColor(
	InterfaceElement inputField,
	LinearColor color)
{
	assert(inputField);
	UiInputFieldHandle handle =
		getInterfaceElementHandle(inputField);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
	handle->focusedColor = color;
}

uint32_t getUiInputFieldMask(
	InterfaceElement inputField)
{
	assert(inputField);
	UiInputFieldHandle handle =
		getInterfaceElementHandle(inputField);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
	return handle->mask;
}
void setUiInputFieldMask(
	InterfaceElement inputField,
	uint32_t mask)
{
	assert(inputField);
	UiInputFieldHandle handle =
		getInterfaceElementHandle(inputField);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
	Text text = getTextRenderText(handle->textRender);

	if (getTextLength(text) > 0)
	{
		bakeUiInputFieldText(text, mask, getFontAtlasLogger(
			handle->ui->fontAtlas));
	}

	handle->mask = mask;
}

const uint32_t* getUiInputFieldText(
	InterfaceElement inputField)
{
	assert(inputField);
	UiInputFieldHandle handle =
		getInterfaceElementHandle(inputField);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
	Text text = getTextRenderText(
		handle->textRender);
	return getTextString(text);
}
size_t getUiInputFieldTextLength(
	InterfaceElement inputField)
{
	assert(inputField);
	UiInputFieldHandle handle =
		getInterfaceElementHandle(inputField);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
	Text text = getTextRenderText(
		handle->textRender);
	return getTextLength(text);
}

bool setUiInputFieldText(
	InterfaceElement inputField,
	const uint32_t* string,
	size_t length)
{
	assert(inputField);
	assert(length == 0 ||
		(length > 0 && string));
	UiInputFieldHandle handle =
		getInterfaceElementHandle(inputField);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
	Text text = getTextRenderText(handle->textRender);
	UserInterface ui = handle->ui;

	if (length > 0)
	{
		bool result = setTextString(text, string, length);

		if (!result)
			return false;

		bakeUiInputFieldText(text,
			handle->mask,
			getFontAtlasLogger(ui->fontAtlas));
		setTransformActive(getGraphicsRenderTransform(
			handle->textRender), true);
		setTransformActive(getGraphicsRenderTransform(
			handle->placeholderRender), false);
	}
	else
	{
		setTransformActive(getGraphicsRenderTransform(
			handle->textRender), false);
		setTransformActive(getGraphicsRenderTransform(
			handle->placeholderRender), true);
	}

	if (ui->focusedInputField == inputField)
	{
		handle->ui->cursorIndex = length;
		Transform textTransform = getGraphicsRenderTransform(
			getTextLength(text) > 0 ?
			handle->textRender : handle->placeholderRender);
		updateUiCursor(ui, textTransform, text);
	}

	return true;
}
bool setUiInputFieldText8(
	InterfaceElement inputField,
	const char* string,
	size_t length)
{
	assert(inputField);
	assert(length == 0 ||
		(length > 0 && string));
	UiInputFieldHandle handle =
		getInterfaceElementHandle(inputField);
	assert(handle->type == INPUT_FIELD_UI_TYPE);
	Text text = getTextRenderText(handle->textRender);
	UserInterface ui = handle->ui;

	if (length > 0)
	{
		bool result = setTextString8(text, string, length);

		if (!result)
			return false;

		bakeUiInputFieldText(text,
			handle->mask,
			getFontAtlasLogger(ui->fontAtlas));
		setTransformActive(getGraphicsRenderTransform(
			handle->textRender), true);
		setTransformActive(getGraphicsRenderTransform(
			handle->placeholderRender), false);
	}
	else
	{
		setTransformActive(getGraphicsRenderTransform(
			handle->textRender), false);
		setTransformActive(getGraphicsRenderTransform(
			handle->placeholderRender), true);
	}

	if (ui->focusedInputField == inputField)
	{
		handle->ui->cursorIndex = length;
		Transform textTransform = getGraphicsRenderTransform(
			getTextLength(text) > 0 ?
			handle->textRender : handle->placeholderRender);
		updateUiCursor(ui, textTransform, text);
	}

	return true;
}
