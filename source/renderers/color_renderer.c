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

#include "uran/renderers/color_renderer.h"

#include <string.h>
#include <assert.h>

typedef struct Handle_T
{
	LinearColor color;
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
	setColorPipelineMvp(
		graphicsPipeline,
		mvp);
	setColorPipelineColor(
		graphicsPipeline,
		handle->color);
	return drawGraphicsMesh(
		graphicsPipeline,
		handle->mesh);
}
GraphicsRenderer createColorRenderer(
	GraphicsPipeline colorPipeline,
	GraphicsRenderSorting sorting,
	bool useCulling,
	size_t capacity,
	ThreadPool threadPool)
{
	assert(colorPipeline);
	assert(sorting < GRAPHICS_RENDER_SORTING_COUNT);
	assert(capacity > 0);

	assert(strcmp(getGraphicsPipelineName(
		colorPipeline),
		COLOR_PIPELINE_NAME) == 0);

	return createGraphicsRenderer(
		colorPipeline,
		sorting,
		useCulling,
		onDestroy,
		onDraw,
		capacity,
		threadPool);
}
GraphicsRender createColorRender(
	GraphicsRenderer colorRenderer,
	Transform transform,
	Box3F bounding,
	LinearColor color,
	GraphicsMesh mesh)
{
	assert(colorRenderer);
	assert(transform);
	assert(mesh);

	assert(getGraphicsPipelineWindow(
		getGraphicsRendererPipeline(
		colorRenderer)) ==
		getGraphicsMeshWindow(mesh));
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		colorRenderer)),
		COLOR_PIPELINE_NAME) == 0);

	Handle handle = calloc(1, sizeof(Handle_T));

	if (!handle)
		return NULL;

	handle->color = color;
	handle->mesh = mesh;

	GraphicsRender render = createGraphicsRender(
		colorRenderer,
		transform,
		bounding,
		handle);

	if (!render)
	{
		onDestroy(handle);
		return NULL;
	}

	return render;
}

LinearColor getColorRenderColor(
	GraphicsRender colorRender)
{
	assert(colorRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		colorRender))),
		COLOR_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		colorRender);
	return handle->color;
}
void setColorRenderColor(
	GraphicsRender colorRender,
	LinearColor color)
{
	assert(colorRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		colorRender))),
		COLOR_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		colorRender);
	handle->color = color;
}

GraphicsMesh getColorRenderMesh(
	GraphicsRender colorRender)
{
	assert(colorRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		colorRender))),
		COLOR_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		colorRender);
	return handle->mesh;
}
void setColorRenderMesh(
	GraphicsRender colorRender,
	GraphicsMesh mesh)
{
	assert(colorRender);
	assert(mesh);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		colorRender))),
		COLOR_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		colorRender);
	handle->mesh = mesh;
}
