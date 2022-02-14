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

#include "uran/renderers/text_renderer.h"
#include "mpgx/_source/graphics_pipeline.h"

#include <string.h>
#include <assert.h>

typedef struct Handle_T
{
	Text text;
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

	Vec2I framebufferSize = graphicsPipeline->base.framebuffer->base.size;
	Vec2I halfFramebufferSize = divValVec2I(framebufferSize, 2);
	Handle handle = getGraphicsRenderHandle(graphicsRender);
	Mat4F mvp = dotMat4F(*viewProj, *model);
	Vec3F position = getTranslationMat4F(mvp);
	position.x = cmmtFloor(position.x *
		(cmmt_float_t)halfFramebufferSize.x) /
		(cmmt_float_t)halfFramebufferSize.x;
	position.y = cmmtFloor(position.y *
		(cmmt_float_t)halfFramebufferSize.y) /
		(cmmt_float_t)halfFramebufferSize.y;
	mvp = setTranslationMat4F(mvp, position);
	setTextPipelineMVP(graphicsPipeline, &mvp);
	setTextPipelineColor(graphicsPipeline, handle->color);

	Vec4I stateScissor = graphicsPipeline->base.state.scissor;
	bool dynamicScissor = stateScissor.z + stateScissor.w == 0;

	if (dynamicScissor)
	{
		Vec4I panelScissor = handle->scissor;
		assert(panelScissor.x + panelScissor.z <= framebufferSize.x);
		assert(panelScissor.y + panelScissor.w <= framebufferSize.y);
		setWindowScissor(graphicsPipeline->base.window, panelScissor);
	}

	return drawText(handle->text);
}
GraphicsRenderer createTextRenderer(
	GraphicsPipeline textPipeline,
	GraphicsRenderSorting sorting,
	bool useCulling,
	size_t capacity,
	ThreadPool threadPool)
{
	assert(textPipeline);
	assert(sorting < GRAPHICS_RENDER_SORTING_COUNT);
	assert(capacity > 0);

	assert(strcmp(getGraphicsPipelineName(
		textPipeline),
		TEXT_PIPELINE_NAME) == 0);

	return createGraphicsRenderer(
		textPipeline,
		sorting,
		useCulling,
		onDestroy,
		onDraw,
		capacity,
		threadPool);
}

GraphicsRender createTextRender(
	GraphicsRenderer textRenderer,
	Transform transform,
	Box3F bounds,
	LinearColor color,
	Text text,
	Vec4I scissor)
{
	assert(textRenderer);
	assert(transform);
	assert(text);
	assert(scissor.x >= 0);
	assert(scissor.y >= 0);
	assert(scissor.z >= 0);
	assert(scissor.w >= 0);

	assert(getGraphicsPipelineWindow(
		getGraphicsRendererPipeline(
		textRenderer)) ==
		getGraphicsPipelineWindow(
		getFontAtlasPipeline(
		getTextFontAtlas(text))));
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		textRenderer)),
		TEXT_PIPELINE_NAME) == 0);

	Handle handle = calloc(1, sizeof(Handle_T));

	if (!handle)
		return NULL;

	handle->text = text;
	handle->color = color;
	handle->scissor = scissor;

	GraphicsRender render = createGraphicsRender(
		textRenderer,
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

LinearColor getTextRenderColor(
	GraphicsRender textRender)
{
	assert(textRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		textRender))),
		TEXT_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		textRender);
	return handle->color;
}
void setTextRenderColor(
	GraphicsRender textRender,
	LinearColor color)
{
	assert(textRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		textRender))),
		TEXT_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		textRender);
	handle->color = color;
}

Vec4I getTextRenderScissor(
	GraphicsRender textRender)
{
	assert(textRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		textRender))),
		TEXT_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		textRender);
	return handle->scissor;
}
void setTextRenderScissor(
	GraphicsRender textRender,
	Vec4I scissor)
{
	assert(textRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		textRender))),
		TEXT_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		textRender);
	handle->scissor = scissor;
}

Text getTextRenderText(
	GraphicsRender textRender)
{
	assert(textRender);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		textRender))),
		TEXT_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		textRender);
	return handle->text;
}
void setTextRenderText(
	GraphicsRender textRender,
	Text text)
{
	assert(textRender);
	assert(text);
	assert(strcmp(getGraphicsPipelineName(
		getGraphicsRendererPipeline(
		getGraphicsRenderRenderer(
		textRender))),
		TEXT_PIPELINE_NAME) == 0);
	Handle handle = getGraphicsRenderHandle(
		textRender);
	handle->text = text;
}
