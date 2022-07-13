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

#include "uran/renderers/diffuse_renderer.h"

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
	Handle handle = getGraphicsRenderHandle(graphicsRender);
	Mat4F mvp = dotMat4F(*viewProj, *model);
	Mat4F normal = transposeMat4F(invMat4F(*model));
	setDiffusePipelineMvp(graphicsPipeline, &mvp);
	setDiffusePipelineNormal(graphicsPipeline, &normal);
	return drawGraphicsMesh(graphicsPipeline, handle->mesh);
}
GraphicsRenderer createDiffuseRenderer(
	GraphicsPipeline diffusePipeline,
	GraphicsRenderSorting sorting,
	bool useCulling,
	size_t capacity,
	ThreadPool threadPool)
{
	assert(diffusePipeline);
	assert(sorting < GRAPHICS_RENDER_SORTING_COUNT);
	assert(capacity > 0);

	assert(strcmp(getGraphicsPipelineName(
		diffusePipeline),
		DIFFUSE_PIPELINE_NAME) == 0);

	return createGraphicsRenderer(
		diffusePipeline,
		sorting,
		useCulling,
		onDestroy,
		onDraw,
		capacity,
		threadPool);
}

GraphicsRender createDiffuseRender(
	GraphicsRenderer diffuseRenderer,
	Transform transform,
	Box3F bounds,
	GraphicsMesh mesh)
{
	assert(diffuseRenderer);
	assert(transform);
	assert(mesh);

	assert(getGraphicsPipelineWindow(
		getGraphicsRendererPipeline(
		diffuseRenderer)) ==
		getGraphicsMeshWindow(mesh));
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		diffuseRenderer)),
		DIFFUSE_PIPELINE_NAME) == 0);

	Handle handle = calloc(1, sizeof(Handle_T));

	if (!handle)
		return NULL;

	handle->mesh = mesh;

	GraphicsRender render = createGraphicsRender(
		diffuseRenderer,
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

GraphicsMesh getDiffuseRenderMesh(
	GraphicsRender diffuseRender)
{
	assert(diffuseRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		diffuseRender))),
		DIFFUSE_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		diffuseRender);
	return handle->mesh;
}
void setDiffuseRenderMesh(
	GraphicsRender diffuseRender,
	GraphicsMesh mesh)
{
	assert(diffuseRender);
	assert(mesh);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		diffuseRender))),
		DIFFUSE_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		diffuseRender);
	handle->mesh = mesh;
}
