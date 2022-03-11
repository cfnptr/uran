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

#include "uran/renderers/texture_color_renderer.h"

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
	setTextureColorPipelineMvp(
		graphicsPipeline,
		mvp);
	setTextureColorPipelineColor(
		graphicsPipeline,
		handle->color);
	setTextureColorPipelineSize(
		graphicsPipeline,
		handle->size);
	setTextureColorPipelineOffset(
		graphicsPipeline,
		handle->offset);
	return drawGraphicsMesh(
		graphicsPipeline,
		handle->mesh);
}
GraphicsRenderer createTextureColorRenderer(
	GraphicsPipeline textureColorPipeline,
	GraphicsRenderSorting sorting,
	bool useCulling,
	size_t capacity,
	ThreadPool threadPool)
{
	assert(textureColorPipeline);
	assert(sorting < GRAPHICS_RENDER_SORTING_COUNT);

	assert(strcmp(getGraphicsPipelineName(
		textureColorPipeline),
		TEXTURE_COLOR_PIPELINE_NAME) == 0);

	return createGraphicsRenderer(
		textureColorPipeline,
		sorting,
		useCulling,
		onDestroy,
		onDraw,
		capacity,
		threadPool);
}
GraphicsRender createTextureColorRender(
	GraphicsRenderer textureColorRenderer,
	Transform transform,
	Box3F bounds,
	LinearColor color,
	Vec2F size,
	Vec2F offset,
	GraphicsMesh mesh)
{
	assert(textureColorRenderer);
	assert(transform);
	assert(mesh);

	assert(getGraphicsPipelineWindow(
		getGraphicsRendererPipeline(
		textureColorRenderer)) ==
		getGraphicsMeshWindow(mesh));
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		textureColorRenderer)),
		TEXTURE_COLOR_PIPELINE_NAME) == 0);

	Handle handle = calloc(1, sizeof(Handle_T));

	if (!handle)
		return NULL;

	handle->color = color;
	handle->size = size;
	handle->offset = offset;
	handle->mesh = mesh;

	GraphicsRender render = createGraphicsRender(
		textureColorRenderer,
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

LinearColor getTexColRenderColor(
	GraphicsRender textureSpriteRender)
{
	assert(textureSpriteRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		textureSpriteRender))),
		TEXTURE_COLOR_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		textureSpriteRender);
	return handle->color;
}
void setTexColRenderColor(
	GraphicsRender textureSpriteRender,
	LinearColor color)
{
	assert(textureSpriteRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		textureSpriteRender))),
		TEXTURE_COLOR_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		textureSpriteRender);
	handle->color = color;
}

Vec2F getTexColRenderSize(
	GraphicsRender textureSpriteRender)
{
	assert(textureSpriteRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		textureSpriteRender))),
		TEXTURE_COLOR_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		textureSpriteRender);
	return handle->size;
}
void setTexColRenderSize(
	GraphicsRender textureSpriteRender,
	Vec2F size)
{
	assert(textureSpriteRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		textureSpriteRender))),
		TEXTURE_COLOR_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		textureSpriteRender);
	handle->size = size;
}

Vec2F getTexColRenderOffset(
	GraphicsRender textureSpriteRender)
{
	assert(textureSpriteRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		textureSpriteRender))),
		TEXTURE_COLOR_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		textureSpriteRender);
	return handle->offset;
}
void setTexColRenderOffset(
	GraphicsRender textureSpriteRender,
	Vec2F offset)
{
	assert(textureSpriteRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		textureSpriteRender))),
		TEXTURE_COLOR_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		textureSpriteRender);
	handle->offset = offset;
}

GraphicsMesh getTexColRenderMesh(
	GraphicsRender textureSpriteRender)
{
	assert(textureSpriteRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		textureSpriteRender))),
		TEXTURE_COLOR_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		textureSpriteRender);
	return handle->mesh;
}
void setTexColRenderMesh(
	GraphicsRender textureSpriteRender,
	GraphicsMesh mesh)
{
	assert(textureSpriteRender);
	assert(mesh);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		textureSpriteRender))),
		TEXTURE_COLOR_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		textureSpriteRender);
	handle->mesh = mesh;
}
