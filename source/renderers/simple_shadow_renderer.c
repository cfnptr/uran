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

#include "uran/renderers/simple_shadow_renderer.h"

#include <string.h>
#include <assert.h>

typedef struct Handle_T
{
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
	setSimpleShadowPipelineMvp(
		graphicsPipeline,
		mvp);
	return drawGraphicsMesh(
		graphicsPipeline,
		handle->mesh);
}
GraphicsRenderer createSimpleShadowRenderer(
	GraphicsPipeline simpleShadowPipeline,
	GraphicsRenderSorting sorting,
	bool useCulling,
	size_t capacity,
	ThreadPool threadPool)
{
	assert(simpleShadowPipeline);
	assert(sorting < GRAPHICS_RENDER_SORTING_COUNT);
	assert(capacity > 0);

	assert(strcmp(getGraphicsPipelineName(
		simpleShadowPipeline),
		SIMPLE_SHADOW_PIPELINE_NAME) == 0);

	return createGraphicsRenderer(
		simpleShadowPipeline,
		sorting,
		useCulling,
		onDestroy,
		onDraw,
		capacity,
		threadPool);
}
GraphicsRender createSimpleShadowRender(
	GraphicsRenderer simpleShadowRenderer,
	Transform transform,
	Box3F bounding,
	GraphicsMesh mesh)
{
	assert(simpleShadowRenderer);
	assert(transform);
	assert(mesh);

	assert(getGraphicsPipelineWindow(
		getGraphicsRendererPipeline(
		simpleShadowRenderer)) ==
		getGraphicsMeshWindow(mesh));
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		simpleShadowRenderer)),
		SIMPLE_SHADOW_PIPELINE_NAME) == 0);

	Handle handle = calloc(1, sizeof(Handle_T));

	if (!handle)
		return NULL;

	handle->mesh = mesh;

	GraphicsRender render = createGraphicsRender(
		simpleShadowRenderer,
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

GraphicsMesh getSimpleShadowRenderMesh(
	GraphicsRender simpleShadowRender)
{
	assert(simpleShadowRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		simpleShadowRender))),
		SIMPLE_SHADOW_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		simpleShadowRender);
	return handle->mesh;
}
void setSimpleShadowRenderMesh(
	GraphicsRender simpleShadowRender,
	GraphicsMesh mesh)
{
	assert(simpleShadowRender);
	assert(mesh);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		simpleShadowRender))),
		SIMPLE_SHADOW_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		simpleShadowRender);
	handle->mesh = mesh;
}
