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

typedef UiPanelHandle_T* UiPanelHandle;

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

	UserInterface userInterface = calloc(1,
		sizeof(UserInterface_T));

	if (!userInterface)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	userInterface->transformer = transformer;

	Window window = getGraphicsPipelineWindow(spritePipeline);

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

void preUpdateUserInterface(UserInterface ui)
{
	assert(ui);
	preUpdateInterface(ui->interface);
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
