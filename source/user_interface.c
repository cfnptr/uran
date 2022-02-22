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
#include "uran/primitives/square_primitive.h"

#if _WIN32
#undef interface
#endif

struct UserInterface_T
{
	Window window;
	Transformer transformer;
	Interface interface;
	GraphicsMesh squareMesh;
	GraphicsRenderer spriteRenderer;
	GraphicsRenderer textRenderer;
};

typedef struct UiPanelHandle_T
{
	Transform transform;
	GraphicsRender render;
} UiPanelHandle_T;
typedef struct UiWindowHandle_T
{
	Window window;
	Transform barTransform;
	GraphicsRender barRender;
	Transform panelTransform;
	GraphicsRender panelRender;
	Vec2F lastCursorPosition;
	bool isDragging;
} UiWindowHandle_T;
typedef struct UiButtonHandle_T
{
	LinearColor disabledColor;
	LinearColor enabledColor;
	LinearColor hoveredColor;
	LinearColor pressedColor;
	Transform transform;
	GraphicsRender render;
} UiButtonHandle_T;

typedef UiPanelHandle_T* UiPanelHandle;
typedef UiWindowHandle_T* UiWindowHandle;
typedef UiButtonHandle_T* UiButtonHandle;

inline static MpgxResult createSquareMesh(
	Window window,
	GraphicsMesh* squareMesh)
{
	assert(window);
	assert(squareMesh);

	Buffer vertexBuffer;

	MpgxResult mpgxResult = createBuffer(
		window,
		VERTEX_BUFFER_TYPE,
		GPU_ONLY_BUFFER_USAGE,
		oneSquareVertices2D,
		sizeof(oneSquareVertices2D),
		&vertexBuffer);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	Buffer indexBuffer;

	mpgxResult = createBuffer(
		window,
		INDEX_BUFFER_TYPE,
		GPU_ONLY_BUFFER_USAGE,
		triangleSquareIndices,
		sizeof(triangleSquareIndices),
		&indexBuffer);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyBuffer(vertexBuffer);
		return mpgxResult;
	}

	GraphicsMesh mesh;

	mpgxResult = createGraphicsMesh(
		window,
		UINT16_INDEX_TYPE,
		sizeof(triangleSquareIndices) / sizeof(uint16_t),
		0,
		vertexBuffer,
		indexBuffer,
		&mesh);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyBuffer(vertexBuffer);
		destroyBuffer(indexBuffer);
		return mpgxResult;
	}

	*squareMesh = mesh;
	return SUCCESS_MPGX_RESULT;
}
inline static void destroySquareMesh(GraphicsMesh squareMesh)
{
	if (!squareMesh)
		return;

	Buffer vertexBuffer = getGraphicsMeshVertexBuffer(squareMesh);
	Buffer indexBuffer = getGraphicsMeshIndexBuffer(squareMesh);
	destroyGraphicsMesh(squareMesh);
	destroyBuffer(indexBuffer);
	destroyBuffer(vertexBuffer);
}

MpgxResult createUserInterface(
	Transformer transformer,
	GraphicsPipeline spritePipeline,
	GraphicsPipeline textPipeline,
	size_t capacity,
	ThreadPool threadPool,
	UserInterface* ui)
{
	assert(transformer);
	assert(spritePipeline);
	assert(textPipeline);
	assert(ui);

	Window window = getGraphicsPipelineWindow(spritePipeline);

	UserInterface userInterface = calloc(1,
		sizeof(UserInterface_T));

	if (!userInterface)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	userInterface->window = window;
	userInterface->transformer = transformer;

	Interface interface = createInterface(
		window,
		1.0f,
		capacity);

	if (!interface)
	{
		destroyUserInterface(userInterface);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	userInterface->interface = interface;

	GraphicsRenderer spriteRenderer = createSpriteRenderer(
		spritePipeline,
		DESCENDING_GRAPHICS_RENDER_SORTING,
		false,
		0,
		threadPool);

	if (!spriteRenderer)
	{
		destroyUserInterface(userInterface);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	userInterface->spriteRenderer = spriteRenderer;
	userInterface->textRenderer = NULL; // TODO:

	GraphicsMesh squareMesh;

	MpgxResult mpgxResult = createSquareMesh(
		window,
		&squareMesh);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyUserInterface(userInterface);
		return mpgxResult;
	}

	userInterface->squareMesh = squareMesh;

	*ui = userInterface;
	return SUCCESS_MPGX_RESULT;
}
void destroyUserInterface(UserInterface ui)
{
	if (!ui)
		return;

	destroySquareMesh(ui->squareMesh);
	destroyGraphicsRenderer(ui->textRenderer);
	destroyGraphicsRenderer(ui->spriteRenderer);
	destroyInterface(ui->interface);
	free(ui);
}

Transformer getUserInterfaceTransformer(UserInterface ui)
{
	assert(ui);
	return ui->transformer;
}
Interface getUserInterface(UserInterface ui)
{
	assert(ui);
	return ui->interface;
}
GraphicsPipeline getUserInterfaceSpritePipeline(UserInterface ui)
{
	assert(ui);
	return getGraphicsRendererPipeline(ui->spriteRenderer);
}
GraphicsPipeline getUserInterfaceTextPipeline(UserInterface ui)
{
	assert(ui);
	return getGraphicsRendererPipeline(ui->textRenderer);
}

void updateUserInterface(UserInterface ui)
{
	assert(ui);
	updateInterface(ui->interface);
}
GraphicsRendererResult drawUserInterface(UserInterface ui)
{
	assert(ui);

	GraphicsRendererResult result =
		createGraphicsRendererResult();
	GraphicsRendererResult tmpResult;

	GraphicsRendererData data = createGraphicsRenderData(
		identMat4F,
		createInterfaceCamera(ui->interface),
		false);

	tmpResult = drawGraphicsRenderer(
		ui->spriteRenderer,
		&data);
	result = addGraphicsRendererResult(result, tmpResult);
	// TODO: text pipeline
	return result;
}

static void onUiPanelDestroy(void* _handle)
{
	assert(_handle);
	UiPanelHandle handle = (UiPanelHandle)_handle;
	destroyGraphicsRender(handle->render);
	destroyTransform(handle->transform);
}
UiPanel createUiPanel(
	UserInterface ui,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	LinearColor color,
	Transform parent,
	bool isActive)
{
	assert(ui);
	assert(alignment < ALIGNMENT_TYPE_COUNT);

	UiPanelHandle handle = calloc(1,
		sizeof(UiPanelHandle_T));

	if (!handle)
		return NULL;

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
		return NULL;
	}

	handle->transform = transform;

	GraphicsRender render = createSpriteRender(
		ui->spriteRenderer,
		transform,
		oneSizeBox3F,
		color,
		ui->squareMesh);

	if (!render)
	{
		onUiPanelDestroy(handle);
		return NULL;
	}

	handle->render = render;

	InterfaceElementEvents events = {
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	};

	InterfaceElement element = createInterfaceElement(
		ui->interface,
		transform,
		alignment,
		position,
		oneSizeBox2F,
		false,
		onUiPanelDestroy,
		&events,
		handle);

	if (!element)
	{
		onUiPanelDestroy(handle);
		return NULL;
	}

	return element;
}

static void onUiWindowClick(InterfaceElement element)
{
	assert(element);

	UiWindowHandle handle = (UiWindowHandle)
		getInterfaceElementHandle(element);

	if (!handle->isDragging)
	{
		handle->lastCursorPosition =
			getWindowCursorPosition(handle->window);
		handle->isDragging = true;
	}
}
static void onUiWindowUpdate(InterfaceElement element)
{
	assert(element);

	UiWindowHandle handle = (UiWindowHandle)
		getInterfaceElementHandle(element);

	if (handle->isDragging)
	{
		Window window = handle->window;

		if (!getWindowMouseButton(window, LEFT_MOUSE_BUTTON))
		{
			handle->isDragging = false;
			return;
		}

		Vec2F cursorPosition = getWindowCursorPosition(
			handle->window);
		Vec2F offset = subVec2F(
			cursorPosition, handle->lastCursorPosition);
		Vec3F position = getInterfaceElementPosition(element);

		position.x += offset.x;
		position.y -= offset.y;

		setInterfaceElementPosition(element, position);
		handle->lastCursorPosition = cursorPosition;
	}
}
static void onUiWindowDestroy(void* _handle)
{
	assert(_handle);
	UiWindowHandle handle = (UiWindowHandle)_handle;
	destroyGraphicsRender(handle->panelRender);
	destroyTransform(handle->panelTransform);
	destroyGraphicsRender(handle->barRender);
	destroyTransform(handle->barTransform);
}
UiWindow createUiWindow(
	UserInterface ui,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	float barHeight,
	LinearColor barColor,
	LinearColor panelColor,
	Transform parent,
	bool isActive)
{
	assert(ui);
	assert(alignment < ALIGNMENT_TYPE_COUNT);

	UiWindowHandle handle = calloc(1,
		sizeof(UiWindowHandle_T));

	if (!handle)
		return NULL;

	handle->window = ui->window;
	handle->lastCursorPosition = zeroVec2F;
	handle->isDragging = false;

	Transformer transformer = ui->transformer;
	GraphicsRenderer spriteRenderer = ui->spriteRenderer;
	GraphicsMesh squareMesh = ui->squareMesh;

	Transform barTransform = createTransform(
		transformer,
		zeroVec3F,
		vec3F(scale.x, barHeight, (cmmt_float_t)1.0),
		oneQuat,
		NO_ROTATION_TYPE,
		parent,
		isActive);

	if (!barTransform)
	{
		onUiWindowDestroy(handle);
		return NULL;
	}

	handle->barTransform = barTransform;

	GraphicsRender barRender = createSpriteRender(
		spriteRenderer,
		barTransform,
		oneSizeBox3F,
		barColor,
		squareMesh);

	if (!barRender)
	{
		onUiWindowDestroy(handle);
		return NULL;
	}

	handle->barRender = barRender;

	Vec3F panelPosition = vec3F(
		position.x,
		position.y -
			(scale.y * (cmmt_float_t)0.5 +
			barHeight * (cmmt_float_t)0.5),
		(cmmt_float_t)0.0);

	Transform panelTransform = createTransform(
		transformer,
		panelPosition,
		vec3F(scale.x, scale.y, (cmmt_float_t)1.0),
		oneQuat,
		NO_ROTATION_TYPE,
		barTransform,
		true);

	if (!panelTransform)
	{
		onUiWindowDestroy(handle);
		return NULL;
	}

	handle->panelTransform = panelTransform;

	GraphicsRender panelRender = createSpriteRender(
		spriteRenderer,
		panelTransform,
		oneSizeBox3F,
		panelColor,
		squareMesh);

	if (!panelRender)
	{
		onUiWindowDestroy(handle);
		return NULL;
	}

	handle->panelRender = panelRender;

	InterfaceElementEvents events = {
		onUiWindowUpdate,
		NULL, NULL, NULL, NULL, NULL,
		onUiWindowClick,
		NULL,
	};

	InterfaceElement element = createInterfaceElement(
		ui->interface,
		barTransform,
		alignment,
		position,
		oneSizeBox2F,
		true,
		onUiWindowDestroy,
		&events,
		handle);

	if (!element)
	{
		onUiWindowDestroy(handle);
		return NULL;
	}

	return element;
}

static void onUiButtonEnter(InterfaceElement element)
{
	assert(element);

	UiButtonHandle handle = (UiButtonHandle)
		getInterfaceElementHandle(element);
	setSpriteRenderColor(
		handle->render,
		handle->hoveredColor);
}
static void onUiButtonExit(InterfaceElement element)
{
	assert(element);

	UiButtonHandle handle = (UiButtonHandle)
		getInterfaceElementHandle(element);
	setSpriteRenderColor(
		handle->render,
		handle->enabledColor);
}
static void onUiButtonPress(InterfaceElement element)
{
	assert(element);

	UiButtonHandle handle = (UiButtonHandle)
		getInterfaceElementHandle(element);
	setSpriteRenderColor(
		handle->render,
		handle->pressedColor);
}
static void onUiButtonRelease(InterfaceElement element)
{
	assert(element);

	UiButtonHandle handle = (UiButtonHandle)
		getInterfaceElementHandle(element);
	setSpriteRenderColor(
		handle->render,
		handle->hoveredColor);
}
static void onUiButtonDestroy(void* _handle)
{
	assert(_handle);
	UiButtonHandle handle = (UiButtonHandle)_handle;
	destroyGraphicsRender(handle->render);
	destroyTransform(handle->transform);
}
UiButton createUiButton(
	UserInterface ui,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	LinearColor disabledColor,
	LinearColor enabledColor,
	LinearColor hoveredColor,
	LinearColor pressedColor,
	Transform parent,
	bool isEnabled,
	bool isActive)
{
	assert(ui);
	assert(alignment < ALIGNMENT_TYPE_COUNT);

	UiButtonHandle handle = calloc(1,
		sizeof(UiButtonHandle_T));

	if (!handle)
		return NULL;

	handle->disabledColor = disabledColor;
	handle->enabledColor = enabledColor;
	handle->hoveredColor = hoveredColor;
	handle->pressedColor = pressedColor;

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
		onUiButtonDestroy(handle);
		return NULL;
	}

	handle->transform = transform;

	GraphicsRender render = createSpriteRender(
		ui->spriteRenderer,
		transform,
		oneSizeBox3F,
		enabledColor,
		ui->squareMesh);

	if (!render)
	{
		onUiButtonDestroy(handle);
		return NULL;
	}

	handle->render = render;

	InterfaceElementEvents events = {
		NULL, NULL, NULL,
		onUiButtonEnter,
		onUiButtonExit,
		NULL,
		onUiButtonPress,
		onUiButtonRelease,
	};

	InterfaceElement element = createInterfaceElement(
		ui->interface,
		transform,
		alignment,
		position,
		oneSizeBox2F,
		isEnabled,
		onUiButtonDestroy,
		&events,
		handle);

	if (!element)
	{
		onUiButtonDestroy(handle);
		return NULL;
	}

	return element;
}
