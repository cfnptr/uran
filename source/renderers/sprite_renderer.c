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

#include "uran/renderers/sprite_renderer.h"

#include <string.h>
#include <assert.h>

typedef struct Handle_T
{
	LinearColor color;
	Vec2F size;
	Vec2F offset;
	GraphicsMesh mesh;
} Handle_T;

typedef Handle_T* Handle;

static void onDestroy(void* handle)
{
	free((Handle)handle);
}
static size_t onDraw(
	GraphicsRender graphicsRender,
	GraphicsPipeline graphicsPipeline,
	const Mat4F* model,
	const Mat4F* viewProj)
{
	assert(graphicsRender);
	assert(graphicsPipeline);
	assert(model);
	assert(viewProj);

	Handle handle = getGraphicsRenderHandle(
		graphicsRender);
	Mat4F mvp = dotMat4F(
		*viewProj,
		*model);
	setSpritePipelineMvp(
		graphicsPipeline,
		mvp);
	setSpritePipelineColor(
		graphicsPipeline,
		handle->color);
	setSpritePipelineSize(
		graphicsPipeline,
		handle->size);
	setSpritePipelineOffset(
		graphicsPipeline,
		handle->offset);
	return drawGraphicsMesh(
		graphicsPipeline,
		handle->mesh);
}
GraphicsRenderer createSpriteRenderer(
	GraphicsPipeline spritePipeline,
	GraphicsRenderSorting sorting,
	bool useCulling,
	size_t capacity,
	ThreadPool threadPool)
{
	assert(spritePipeline);
	assert(sorting < GRAPHICS_RENDER_SORTING_COUNT);
	assert(capacity > 0);

	assert(strcmp(getGraphicsPipelineName(
		spritePipeline),
		SPRITE_PIPELINE_NAME) == 0);

	return createGraphicsRenderer(
		spritePipeline,
		sorting,
		useCulling,
		onDestroy,
		onDraw,
		capacity,
		threadPool);
}
GraphicsRender createSpriteRender(
	GraphicsRenderer spriteRenderer,
	Transform transform,
	Box3F bounds,
	LinearColor color,
	Vec2F size,
	Vec2F offset,
	GraphicsMesh mesh)
{
	assert(spriteRenderer);
	assert(transform);
	assert(mesh);

	assert(getGraphicsPipelineWindow(
		getGraphicsRendererPipeline(
		spriteRenderer)) ==
		getGraphicsMeshWindow(mesh));
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		spriteRenderer)),
		SPRITE_PIPELINE_NAME) == 0);

	Handle handle = calloc(1, sizeof(Handle_T));

	if (!handle)
		return NULL;

	handle->color = color;
	handle->size = size;
	handle->offset = offset;
	handle->mesh = mesh;

	GraphicsRender render = createGraphicsRender(
		spriteRenderer,
		transform,
		bounds,
		handle);

	if (!render)
	{
		onDestroy(handle);
		return NULL;
	}

	return render;
}

LinearColor getSpriteRenderColor(
	GraphicsRender spriteRender)
{
	assert(spriteRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		spriteRender))),
		SPRITE_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		spriteRender);
	return handle->color;
}
void setSpriteRenderColor(
	GraphicsRender spriteRender,
	LinearColor color)
{
	assert(spriteRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		spriteRender))),
		SPRITE_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		spriteRender);
	handle->color = color;
}

Vec2F getSpriteRenderSize(
	GraphicsRender spriteRender)
{
	assert(spriteRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		spriteRender))),
		SPRITE_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		spriteRender);
	return handle->size;
}
void setSpriteRenderSize(
	GraphicsRender spriteRender,
	Vec2F size)
{
	assert(spriteRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		spriteRender))),
		SPRITE_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		spriteRender);
	handle->size = size;
}

Vec2F getSpriteRenderOffset(
	GraphicsRender spriteRender)
{
	assert(spriteRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		spriteRender))),
		SPRITE_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		spriteRender);
	return handle->offset;
}
void setSpriteRenderOffset(
	GraphicsRender spriteRender,
	Vec2F offset)
{
	assert(spriteRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		spriteRender))),
		SPRITE_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		spriteRender);
	handle->offset = offset;
}

GraphicsMesh getSpriteRenderMesh(
	GraphicsRender spriteRender)
{
	assert(spriteRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		spriteRender))),
		SPRITE_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		spriteRender);
	return handle->mesh;
}
void setSpriteRenderMesh(
	GraphicsRender spriteRender,
	GraphicsMesh mesh)
{
	assert(spriteRender);
	assert(mesh);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		spriteRender))),
		SPRITE_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		spriteRender);
	handle->mesh = mesh;
}
