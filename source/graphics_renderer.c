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

#include "uran/graphics_renderer.h"
#include "mpmt/atomic.h"

#include <assert.h>
#include <string.h>

struct GraphicsRender_T
{
	GraphicsRenderer renderer;
	Transform transform;
	void* handle;
	Box3F bounds;
};
typedef struct GraphicsRenderElement
{
	GraphicsRender render;
	Vec3F rendererPosition;
	Vec3F renderPosition;
} GraphicsRenderElement;
struct GraphicsRenderer_T
{
	GraphicsPipeline pipeline;
	OnGraphicsRenderDestroy onDestroy;
	OnGraphicsRenderDraw onDraw;
	GraphicsRender* renders;
	GraphicsRenderElement* renderElements;
	size_t renderCapacity;
	size_t renderCount;
	ThreadPool threadPool;
	GraphicsRenderSorting sorting;
	bool useCulling;
#ifndef NDEBUG
	bool isEnumerating;
#endif
};

GraphicsRenderer createGraphicsRenderer(
	GraphicsPipeline pipeline,
	GraphicsRenderSorting sorting,
	bool useCulling,
	OnGraphicsRenderDestroy onDestroy,
	OnGraphicsRenderDraw onDraw,
	size_t capacity,
	ThreadPool threadPool)
{
	assert(pipeline);
	assert(sorting < GRAPHICS_RENDER_SORTING_COUNT);
	assert(onDestroy);
	assert(onDraw);
	assert(capacity > 0);

	GraphicsRenderer graphicsRenderer = calloc(1,
		sizeof(GraphicsRenderer_T));

	if (!graphicsRenderer)
		return NULL;

	graphicsRenderer->pipeline = pipeline;
	graphicsRenderer->onDestroy = onDestroy;
	graphicsRenderer->onDraw = onDraw;
	graphicsRenderer->threadPool = threadPool;
	graphicsRenderer->sorting = sorting;
	graphicsRenderer->useCulling = useCulling;
#ifndef NDEBUG
	graphicsRenderer->isEnumerating = false;
#endif

	GraphicsRender* renders = malloc(
		sizeof(GraphicsRender) * capacity);

	if (!renders)
	{
		destroyGraphicsRenderer(graphicsRenderer);
		return NULL;
	}

	graphicsRenderer->renders = renders;
	graphicsRenderer->renderCapacity = capacity;
	graphicsRenderer->renderCount = 0;

	GraphicsRenderElement* renderElements = malloc(
		sizeof(GraphicsRenderElement) * capacity);

	if (!renderElements)
	{
		destroyGraphicsRenderer(graphicsRenderer);
		return NULL;
	}

	graphicsRenderer->renderElements = renderElements;
	return graphicsRenderer;
}
void destroyGraphicsRenderer(GraphicsRenderer renderer)
{
	if (!renderer)
		return;

	assert(renderer->renderCount == 0);
	assert(!renderer->isEnumerating);

	free(renderer->renderElements);
	free(renderer->renders);
	free(renderer);
}

GraphicsPipeline getGraphicsRendererPipeline(GraphicsRenderer renderer)
{
	assert(renderer);
	return renderer->pipeline;
}
OnGraphicsRenderDestroy getGraphicsRendererOnDestroy(GraphicsRenderer renderer)
{
	assert(renderer);
	return renderer->onDestroy;
}
OnGraphicsRenderDraw getGraphicsRendererOnDraw(GraphicsRenderer renderer)
{
	assert(renderer);
	return renderer->onDraw;
}
ThreadPool getGraphicsRendererThreadPool(GraphicsRenderer renderer)
{
	assert(renderer);
	return renderer->threadPool;
}
size_t getGraphicsRendererRenderCount(GraphicsRenderer renderer)
{
	assert(renderer);
	return renderer->renderCount;
}

GraphicsRenderSorting getGraphicsRendererSorting(
	GraphicsRenderer renderer)
{
	assert(renderer);
	return renderer->sorting;
}
void setGraphicsRendererSorting(
	GraphicsRenderer renderer,
	GraphicsRenderSorting sorting)
{
	assert(renderer);
	assert(sorting < GRAPHICS_RENDER_SORTING_COUNT);
	renderer->sorting = sorting;
}

bool getGraphicsRendererUseCulling(
	GraphicsRenderer renderer)
{
	assert(renderer);
	return renderer->useCulling;
}
void setGraphicsRendererUseCulling(
	GraphicsRenderer renderer,
	bool useCulling)
{
	assert(renderer);
	renderer->useCulling = useCulling;
}

void enumerateGraphicsRendererItems(
	GraphicsRenderer renderer,
	OnGraphicsRendererItem onItem,
	void* handle)
{
	assert(renderer);
	assert(onItem);

#ifndef NDEBUG
	renderer->isEnumerating = true;
#endif

	GraphicsRender* renders = renderer->renders;
	size_t renderCount = renderer->renderCount;

	for (size_t i = 0; i < renderCount; i++)
		onItem(renders[i], handle);

#ifndef NDEBUG
	renderer->isEnumerating = false;
#endif
}
void destroyAllGraphicsRendererItems(
	GraphicsRenderer renderer,
	bool destroyTransforms)
{
	assert(renderer);
	assert(!renderer->isEnumerating);

	GraphicsRender* renders = renderer->renders;
	size_t renderCount = renderer->renderCount;

	if (renderCount == 0)
		return;

	for (size_t i = 0; i < renderCount; i++)
	{
		GraphicsRender render = renders[i];
		renderer->onDestroy(render->handle);

		if (destroyTransforms)
			destroyTransform(render->transform);

		free(render);
	}

	renderer->renderCount = 0;
}

static int ascendingRenderCompare(
	const void* a,
	const void* b)
{
	// NOTE: a and b should not be NULL!
	// Skipping assertions for debug build speed.

	const GraphicsRenderElement* data =
		(GraphicsRenderElement*)a;

	cmmt_float_t distanceA = distPowVec3F(
		data->rendererPosition,
		data->renderPosition);

	data = (GraphicsRenderElement*)b;

	cmmt_float_t distanceB = distPowVec3F(
		data->rendererPosition,
		data->renderPosition);

	return distanceA > distanceB ? 1 : -1;
}
static int descendingRenderCompare(
	const void* a,
	const void* b)
{
	// NOTE: a and b should not be NULL!
	// Skipping assertions for debug build speed.

	const GraphicsRenderElement* data =
		(GraphicsRenderElement*)a;

	cmmt_float_t distanceA = distPowVec3F(
		data->rendererPosition,
		data->renderPosition);

	data = (GraphicsRenderElement*)b;

	cmmt_float_t distanceB = distPowVec3F(
		data->rendererPosition,
		data->renderPosition);

	return distanceA < distanceB ? 1 : -1;
}

static int uiAscendingRenderCompare(
	const void* a,
	const void* b)
{
	// NOTE: a and b should not be NULL!
	// Skipping assertions for debug build speed.

	const GraphicsRenderElement* data =
		(GraphicsRenderElement*)a;

	cmmt_float_t distanceA =
		data->renderPosition.z -
		data->rendererPosition.z;

	data = (GraphicsRenderElement*)b;

	cmmt_float_t distanceB =
		data->renderPosition.z -
		data->rendererPosition.z;

	return distanceA > distanceB ? 1 : -1;
}
static int uiDescendingRenderCompare(
	const void* a,
	const void* b)
{
	// NOTE: a and b should not be NULL!
	// Skipping assertions for debug build speed.

	const GraphicsRenderElement* data =
		(GraphicsRenderElement*)a;

	cmmt_float_t distanceA =
		data->renderPosition.z -
		data->rendererPosition.z;

	data = (GraphicsRenderElement*)b;

	cmmt_float_t distanceB =
		data->renderPosition.z -
		data->rendererPosition.z;

	return distanceA < distanceB ? 1 : -1;
}

inline static bool isShouldDraw(
	GraphicsRender render,
	Plane3F leftPlane,
	Plane3F rightPlane,
	Plane3F bottomPlane,
	Plane3F topPlane,
	Plane3F backPlane,
	Plane3F frontPlane,
	Vec3F rendererPosition,
	bool useCulling,
	GraphicsRenderElement* element)
{
	Transform transform = render->transform;

	if (!isTransformActive(transform))
		return false;

	Transform parent = getTransformParent(transform);

	while (parent)
	{
		if (!isTransformActive(parent))
			return false;
		parent = getTransformParent(parent);
	}

	Mat4F model = getTransformModel(transform);
	Vec3F renderPosition = getTranslationMat4F(model);

	if (useCulling)
	{
		Vec3F renderScale = getTransformScale(transform);
		Box3F renderBounds = render->bounds;

		renderBounds.minimum = mulVec3F(
			renderBounds.minimum,
			renderScale);
		renderBounds.minimum = addVec3F(
			renderBounds.minimum,
			renderPosition);
		renderBounds.maximum = mulVec3F(
			renderBounds.maximum,
			renderScale);
		renderBounds.maximum = addVec3F(
			renderBounds.maximum,
			renderPosition);

		bool isInFrustum = isBoxInFrustum3F(
			leftPlane,
			rightPlane,
			bottomPlane,
			topPlane,
			backPlane,
			frontPlane,
			renderBounds);

		if (!isInFrustum)
			return false;
	}

	GraphicsRenderElement renderElement = {
		render,
		rendererPosition,
		renderPosition,
	};

	*element = renderElement;
	return true;
}

typedef struct UpdateData
{
	GraphicsRenderer renderer;
	const GraphicsRendererData* data;
	atomic_int64 threadIndex;
	atomic_int64 elementIndex;
} UpdateData;
static void onRendererDraw(void* argument)
{
	assert(argument);

	UpdateData* updateData = (UpdateData*)argument;
	GraphicsRenderer renderer = updateData->renderer;
	GraphicsRender* renders = renderer->renders;
	GraphicsRenderElement* renderElements =  renderer->renderElements;
	bool useCulling = renderer->useCulling;
	size_t renderCount = renderer->renderCount;
	const GraphicsRendererData* data = updateData->data;

	Vec3F rendererPosition = negVec3F(
		getTranslationMat4F(data->view));
	Plane3F leftPlane = data->leftPlane;
	Plane3F rightPlane = data->rightPlane;
	Plane3F bottomPlane = data->bottomPlane;
	Plane3F topPlane = data->topPlane;
	Plane3F backPlane = data->backPlane;
	Plane3F frontPlane = data->frontPlane;

	size_t threadCount = getThreadPoolThreadCount(
		renderer->threadPool);
	size_t threadIndex = (size_t)atomicFetchAdd64(
		&updateData->threadIndex, 1);
	atomic_int64* elementIndex = &updateData->elementIndex;

	for (size_t i = threadIndex; i < renderCount; i += threadCount)
	{
		GraphicsRender render = renders[i];
		GraphicsRenderElement element;

		bool shouldDraw = isShouldDraw(
			render,
			leftPlane,
			rightPlane,
			bottomPlane,
			topPlane,
			backPlane,
			frontPlane,
			rendererPosition,
			useCulling,
			&element);

		if (!shouldDraw)
			continue;

		size_t index = (size_t)atomicFetchAdd64(elementIndex, 1);
		renderElements[index] = element;
	}
}
GraphicsRendererResult drawGraphicsRenderer(
	GraphicsRenderer renderer,
	const GraphicsRendererData* data)
{
	assert(renderer);
	assert(data);
	assert(!renderer->isEnumerating);

	GraphicsRendererResult result;
	result.drawCount = 0;
	result.indexCount = 0;
	result.passCount = 0;

	size_t renderCount = renderer->renderCount;

	if (!renderCount)
		return result;

	Vec3F rendererPosition = negVec3F(
		getTranslationMat4F(data->view));

	GraphicsRenderElement* renderElements = renderer->renderElements;
	ThreadPool threadPool = renderer->threadPool;

	size_t elementCount = 0;

	if (threadPool && renderCount >= getThreadPoolThreadCount(threadPool))
	{
		size_t threadCount = getThreadPoolThreadCount(threadPool);

		UpdateData updateData = {
			renderer,
			data,
			0,
			0,
		};
		ThreadPoolTask task = {
			onRendererDraw,
			&updateData,
		};
		addThreadPoolTaskNumber(
			threadPool,
			task,
			threadCount);
		waitThreadPool(threadPool);
		elementCount = updateData.elementIndex;
	}
	else
	{
		GraphicsRender* renders = renderer->renders;
		bool useCulling = renderer->useCulling;
		Plane3F leftPlane = data->leftPlane;
		Plane3F rightPlane = data->rightPlane;
		Plane3F bottomPlane = data->bottomPlane;
		Plane3F topPlane = data->topPlane;
		Plane3F backPlane = data->backPlane;
		Plane3F frontPlane = data->frontPlane;

		for (size_t i = 0; i < renderCount; i++)
		{
			GraphicsRender render = renders[i];
			GraphicsRenderElement element;

			bool shouldDraw = isShouldDraw(
				render,
				leftPlane,
				rightPlane,
				bottomPlane,
				topPlane,
				backPlane,
				frontPlane,
				rendererPosition,
				useCulling,
				&element);

			if (!shouldDraw)
				continue;

			renderElements[elementCount++] = element;
		}
	}

	if (elementCount == 0)
		return result;

	GraphicsRenderSorting sorting = renderer->sorting;

	if (sorting != NO_GRAPHICS_RENDER_SORTING && elementCount > 1)
	{
		if (sorting == ASCENDING_GRAPHICS_RENDER_SORTING)
		{
			qsort(renderElements,
				elementCount,
				sizeof(GraphicsRenderElement),
				ascendingRenderCompare);
		}
		else if (sorting == DESCENDING_GRAPHICS_RENDER_SORTING)
		{
			qsort(renderElements,
				elementCount,
				sizeof(GraphicsRenderElement),
				descendingRenderCompare);
		}
		else if (sorting == UI_ASCENDING_GRAPHICS_RENDER_SORTING)
		{
			qsort(renderElements,
				elementCount,
				sizeof(GraphicsRenderElement),
				uiAscendingRenderCompare);
		}
		else if (sorting == UI_DESCENDING_GRAPHICS_RENDER_SORTING)
		{
			qsort(renderElements,
				elementCount,
				sizeof(GraphicsRenderElement),
				uiDescendingRenderCompare);
		}
		else
		{
			abort();
		}
	}

	Mat4F viewProj = data->viewProj;
	GraphicsPipeline pipeline = renderer->pipeline;
	OnGraphicsRenderDraw onDraw = renderer->onDraw;

	// TODO: also multi-thread this code,
	// this is possible with Vulkan multiple command buffers

	bindGraphicsPipeline(pipeline);

	for (size_t i = 0; i < elementCount; i++)
	{
		GraphicsRender render = renderElements[i].render;

		Mat4F model = getTransformModel(
			render->transform);

		size_t indexCount = onDraw(
			render,
			pipeline,
			&model,
			&viewProj);

		if (indexCount > 0)
		{
			result.drawCount++;
			result.indexCount += indexCount;
		}
	}

	return result;
}

GraphicsRender createGraphicsRender(
	GraphicsRenderer renderer,
	Transform transform,
	Box3F bounds,
	void* handle)
{
	assert(renderer);
	assert(transform);
	assert(handle);
	assert(!renderer->isEnumerating);

	GraphicsRender graphicsRender = malloc(
		sizeof(GraphicsRender_T));

	if (!graphicsRender)
		return NULL;

	graphicsRender->renderer = renderer;
	graphicsRender->transform = transform;
	graphicsRender->handle = handle;
	graphicsRender->bounds = bounds;

	size_t count = renderer->renderCount;

	if (count == renderer->renderCapacity)
	{
		size_t capacity = renderer->renderCapacity * 2;

		GraphicsRender* renders = realloc(
			renderer->renders,
			sizeof(GraphicsRender) * capacity);

		if (!renders)
		{
			free(graphicsRender);
			return NULL;
		}

		renderer->renders = renders;

		GraphicsRenderElement* renderElements = realloc(
			renderer->renderElements,
			sizeof(GraphicsRenderElement) * capacity);

		if (!renderElements)
		{
			free(graphicsRender);
			return NULL;
		}

		renderer->renderElements = renderElements;
		renderer->renderCapacity = capacity;
	}

	renderer->renders[count] = graphicsRender;
	renderer->renderCount = count + 1;
	return graphicsRender;
}
void destroyGraphicsRender(GraphicsRender render)
{
	if (!render)
		return;

	assert(!render->renderer->isEnumerating);

	GraphicsRenderer renderer = render->renderer;
	GraphicsRender* renders = renderer->renders;
	size_t renderCount = renderer->renderCount;

	for (int64_t i = (int64_t)renderCount - 1; i >= 0 ; i--)
	{
		if (renders[i] != render)
			continue;

		for (size_t j = i + 1; j < renderCount; j++)
			renders[j - 1] = renders[j];

		renderer->onDestroy(render->handle);

		free(render);
		renderer->renderCount--;
		return;
	}

	abort();
}

void* getGraphicsRenderHandle(GraphicsRender render)
{
	assert(render);
	return render->handle;
}
GraphicsRenderer getGraphicsRenderRenderer(GraphicsRender render)
{
	assert(render);
	return render->renderer;
}
Transform getGraphicsRenderTransform(GraphicsRender render)
{
	assert(render);
	return render->transform;
}

Box3F getGraphicsRenderBounds(
	GraphicsRender render)
{
	assert(render);
	return render->bounds;
}
void setGraphicsRenderBounds(
	GraphicsRender render,
	Box3F bounds)
{
	assert(render);
	render->bounds = bounds;
}
