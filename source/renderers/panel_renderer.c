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

#include "uran/renderers/panel_renderer.h"

#include "mpgx/_source/window.h"
#include "mpgx/_source/graphics_mesh.h"

#include <string.h>
#include <assert.h>

typedef struct Handle_T
{
	LinearColor color;
	Vec4I scissor;
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

	GraphicsMesh mesh = getPanelPipelineMesh(graphicsPipeline);
	Handle handle = getGraphicsRenderHandle(graphicsRender);
	Mat4F mvp = dotMat4F(*viewProj, *model);
	setPanelPipelineMvp(graphicsPipeline, &mvp);
	setPanelPipelineColor(graphicsPipeline, handle->color);

	Vec4I stateScissor = graphicsPipeline->base.state.scissor;
	bool dynamicScissor = stateScissor.z + stateScissor.w == 0;

	if (dynamicScissor)
	{
		Vec4I panelScissor = handle->scissor;
		Vec2I framebufferSize = graphicsPipeline->base.framebuffer->base.size;
		assert(panelScissor.x + panelScissor.z <= framebufferSize.x);
		assert(panelScissor.y + panelScissor.w <= framebufferSize.y);
		setWindowScissor(graphicsPipeline->base.window, panelScissor);
	}

	GraphicsAPI api = getGraphicsAPI();

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = getVkWindow(
			graphicsPipeline->vk.window);

		if (graphicsPipeline->vk.onUniformsSet)
			graphicsPipeline->vk.onUniformsSet(graphicsPipeline);

		vkCmdDrawIndexed(
			vkWindow->currenCommandBuffer,
			(uint32_t)mesh->vk.indexCount,
			1,
			0,
			0,
			0);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		if (graphicsPipeline->gl.onUniformsSet)
			graphicsPipeline->gl.onUniformsSet(graphicsPipeline);

		glDrawElements(
			graphicsPipeline->gl.drawMode,
			(GLsizei)mesh->gl.indexCount,
			mesh->gl.glIndexType,
			(const void*)mesh->gl.glIndexOffset);
		assertOpenGL();
#else
		abort();
#endif
	}
	else
	{
		abort();
	}

	return mesh->base.indexCount;
}
GraphicsRenderer createPanelRenderer(
	GraphicsPipeline panelPipeline,
	GraphicsRenderSorting sorting,
	bool useCulling,
	size_t capacity,
	ThreadPool threadPool)
{
	assert(panelPipeline);
	assert(sorting < GRAPHICS_RENDER_SORTING_COUNT);
	assert(capacity > 0);

	assert(strcmp(getGraphicsPipelineName(
		panelPipeline),
		PANEL_PIPELINE_NAME) == 0);

	return createGraphicsRenderer(
		panelPipeline,
		sorting,
		useCulling,
		onDestroy,
		onDraw,
		capacity,
		threadPool);
}

GraphicsRender createPanelRender(
	GraphicsRenderer panelRenderer,
	Transform transform,
	Box3F bounds,
	LinearColor color,
	Vec4I scissor)
{
	assert(panelRenderer);
	assert(transform);

	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		panelRenderer)),
		PANEL_PIPELINE_NAME) == 0);

	Handle handle = calloc(1, sizeof(Handle_T));

	if (!handle)
		return NULL;

	handle->color = color;
	handle->scissor = scissor;

	GraphicsRender render = createGraphicsRender(
		panelRenderer,
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

LinearColor getPanelRenderColor(
	GraphicsRender panelRender)
{
	assert(panelRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		panelRender))),
		PANEL_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		panelRender);
	return handle->color;
}
void setPanelRenderColor(
	GraphicsRender panelRender,
	LinearColor color)
{
	assert(panelRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		panelRender))),
		PANEL_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		panelRender);
	handle->color = color;
}

Vec4I getPanelRenderScissor(
	GraphicsRender panelRender)
{
	assert(panelRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		panelRender))),
		PANEL_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		panelRender);
	return handle->scissor;
}
void setPanelRenderScissor(
	GraphicsRender panelRender,
	Vec4I scissor)
{
	assert(panelRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		panelRender))),
		PANEL_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		panelRender);
	handle->scissor = scissor;
}
