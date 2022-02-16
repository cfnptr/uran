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
	Box3F bounding;
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
	const GraphicsRendererData* data;
	ThreadPool threadPool;
	atomic_int64 threadIndex;
	atomic_int64 elementIndex;
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
	graphicsRenderer->data = NULL;
	graphicsRenderer->threadPool = threadPool;
	graphicsRenderer->threadIndex = 0;
	graphicsRenderer->elementIndex = 0;
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
void destroyGraphicsRenderer(
	GraphicsRenderer graphicsRenderer)
{
	if (!graphicsRenderer)
		return;

	assert(graphicsRenderer->renderCount == 0);
	assert(!graphicsRenderer->isEnumerating);

	free(graphicsRenderer->renderElements);
	free(graphicsRenderer->renders);
	free(graphicsRenderer);
}

GraphicsPipeline getGraphicsRendererPipeline(
	GraphicsRenderer graphicsRenderer)
{
	assert(graphicsRenderer);
	return graphicsRenderer->pipeline;
}
OnGraphicsRenderDestroy getGraphicsRendererOnDestroy(
	GraphicsRenderer graphicsRenderer)
{
	assert(graphicsRenderer);
	return graphicsRenderer->onDestroy;
}
OnGraphicsRenderDraw getGraphicsRendererOnDraw(
	GraphicsRenderer graphicsRenderer)
{
	assert(graphicsRenderer);
	return graphicsRenderer->onDraw;
}
ThreadPool getGraphicsRendererThreadPool(
	GraphicsRenderer graphicsRenderer)
{
	assert(graphicsRenderer);
	return graphicsRenderer->threadPool;
}

GraphicsRenderSorting getGraphicsRendererSorting(
	GraphicsRenderer graphicsRenderer)
{
	assert(graphicsRenderer);
	return graphicsRenderer->sorting;
}
void setGraphicsRendererSorting(
	GraphicsRenderer graphicsRenderer,
	GraphicsRenderSorting sorting)
{
	assert(graphicsRenderer);
	assert(sorting < GRAPHICS_RENDER_SORTING_COUNT);
	graphicsRenderer->sorting = sorting;
}

bool getGraphicsRendererUseCulling(
	GraphicsRenderer graphicsRenderer)
{
	assert(graphicsRenderer);
	return graphicsRenderer->useCulling;
}
void setGraphicsRendererUseCulling(
	GraphicsRenderer graphicsRenderer,
	bool useCulling)
{
	assert(graphicsRenderer);
	graphicsRenderer->useCulling = useCulling;
}

size_t getGraphicsRendererRenderCount(
	GraphicsRenderer graphicsRenderer)
{
	assert(graphicsRenderer);
	return graphicsRenderer->renderCount;
}
void enumerateGraphicsRenderer(
	GraphicsRenderer graphicsRenderer,
	void(*onItem)(GraphicsRender))
{
	assert(graphicsRenderer);
	assert(onItem);

#ifndef NDEBUG
	graphicsRenderer->isEnumerating = true;
#endif

	GraphicsRender* renders = graphicsRenderer->renders;
	size_t renderCount = graphicsRenderer->renderCount;

	for (size_t i = 0; i < renderCount; i++)
		onItem(renders[i]);

#ifndef NDEBUG
	graphicsRenderer->isEnumerating = false;
#endif
}
void destroyAllGraphicsRendererRenders(
	GraphicsRenderer graphicsRenderer,
	bool destroyTransforms)
{
	assert(graphicsRenderer);
	assert(!graphicsRenderer->isEnumerating);

	GraphicsRender* renders = graphicsRenderer->renders;
	size_t renderCount = graphicsRenderer->renderCount;

	if (renderCount == 0)
		return;

	for (size_t i = 0; i < renderCount; i++)
	{
		GraphicsRender render = renders[i];
		graphicsRenderer->onDestroy(render->handle);

		if (destroyTransforms)
			destroyTransform(render->transform);

		free(render);
	}

	graphicsRenderer->renderCount = 0;
}

static int ascendingRenderCompare(
	const void* a,
	const void* b)
{
	// NOTE: a and b should not be NULL!
	// Skipping assertions for debug build speed.

	const GraphicsRenderElement* data =
		(GraphicsRenderElement*)a;

	float distanceA = distPowVec3F(
		data->rendererPosition,
		data->renderPosition);

	data = (GraphicsRenderElement*)b;

	float distanceB = distPowVec3F(
		data->rendererPosition,
		data->renderPosition);

	if (distanceA < distanceB)
		return -1;
	if (distanceA == distanceB)
		return 0;
	if (distanceA > distanceB)
		return 1;

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

	float distanceA = distPowVec3F(
		data->rendererPosition,
		data->renderPosition);

	data = (GraphicsRenderElement*)b;

	float distanceB = distPowVec3F(
		data->rendererPosition,
		data->renderPosition);

	return distanceA < distanceB ? 1 : -1;
}

void createGraphicsRenderData(
	Mat4F view,
	Camera camera,
	GraphicsRendererData* graphicsRendererData,
	bool createPlanes)
{
	assert(graphicsRendererData);

	GraphicsAPI api = getGraphicsAPI();

	Mat4F proj;
	Mat4F viewProj;

	if (camera.persp.type == PERSP_CAMERA_TYPE)
	{
		if (api == VULKAN_GRAPHICS_API)
		{
			proj = perspZeroOneMat4F(
				camera.persp.fieldOfView,
				camera.persp.aspectRatio,
				camera.persp.nearClipPlane,
				camera.persp.farClipPlane);
			viewProj = dotMat4F(
				proj, view);

			if (createPlanes)
			{
				frustumZeroOneMat4F(
					viewProj,
					&graphicsRendererData->leftPlane,
					&graphicsRendererData->rightPlane,
					&graphicsRendererData->bottomPlane,
					&graphicsRendererData->topPlane,
					&graphicsRendererData->backPlane,
					&graphicsRendererData->frontPlane,
					false);
			}
		}
		else if (api == OPENGL_GRAPHICS_API ||
			api == OPENGL_ES_GRAPHICS_API)
		{
			proj = perspNegOneMat4F(
				camera.persp.fieldOfView,
				camera.persp.aspectRatio,
				camera.persp.nearClipPlane,
				camera.persp.farClipPlane);
			viewProj = dotMat4F(
				proj, view);

			if (createPlanes)
			{
				frustumNegOneMat4F(
					viewProj,
					&graphicsRendererData->leftPlane,
					&graphicsRendererData->rightPlane,
					&graphicsRendererData->bottomPlane,
					&graphicsRendererData->topPlane,
					&graphicsRendererData->backPlane,
					&graphicsRendererData->frontPlane,
					false);
			}
		}
		else
		{
			abort();
		}
	}
	else if (camera.ortho.type == ORTHO_CAMERA_TYPE)
	{
		if (api == VULKAN_GRAPHICS_API)
		{
			proj = orthoZeroOneMat4F(
				camera.ortho.leftFrustum,
				camera.ortho.rightFrustum,
				camera.ortho.bottomFrustum,
				camera.ortho.topFrustum,
				camera.ortho.nearClipPlane,
				camera.ortho.farClipPlane);
			viewProj = dotMat4F(
				proj, view);

			if (createPlanes)
			{
				frustumZeroOneMat4F(
					viewProj,
					&graphicsRendererData->leftPlane,
					&graphicsRendererData->rightPlane,
					&graphicsRendererData->bottomPlane,
					&graphicsRendererData->topPlane,
					&graphicsRendererData->backPlane,
					&graphicsRendererData->frontPlane,
					false);
			}
		}
		else if (api == OPENGL_GRAPHICS_API ||
			api == OPENGL_ES_GRAPHICS_API)
		{
			proj = orthoNegOneMat4F(
				camera.ortho.leftFrustum,
				camera.ortho.rightFrustum,
				camera.ortho.bottomFrustum,
				camera.ortho.topFrustum,
				camera.ortho.nearClipPlane,
				camera.ortho.farClipPlane);
			viewProj = dotMat4F(
				proj, view);

			if (createPlanes)
			{
				frustumNegOneMat4F(
					viewProj,
					&graphicsRendererData->leftPlane,
					&graphicsRendererData->rightPlane,
					&graphicsRendererData->bottomPlane,
					&graphicsRendererData->topPlane,
					&graphicsRendererData->backPlane,
					&graphicsRendererData->frontPlane,
					false);
			}
		}
		else
		{
			abort();
		}
	}
	else
	{
		abort();
	}

	graphicsRendererData->view = view;
	graphicsRendererData->proj = proj;
	graphicsRendererData->viewProj = viewProj;

	if (!createPlanes)
	{
		graphicsRendererData->leftPlane = plane3F(zeroVec3F, 0.0f);
		graphicsRendererData->rightPlane = plane3F(zeroVec3F, 0.0f);
		graphicsRendererData->bottomPlane = plane3F(zeroVec3F, 0.0f);
		graphicsRendererData->topPlane = plane3F(zeroVec3F, 0.0f);
		graphicsRendererData->backPlane = plane3F(zeroVec3F, 0.0f);
		graphicsRendererData->frontPlane = plane3F(zeroVec3F, 0.0f);
	}
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
		Box3F renderBounding = getGraphicsRenderBounding(render);

		renderBounding.minimum = mulVec3F(
			renderBounding.minimum,
			renderScale);
		renderBounding.minimum = addVec3F(
			renderBounding.minimum,
			renderPosition);
		renderBounding.maximum = mulVec3F(
			renderBounding.maximum,
			renderScale);
		renderBounding.maximum = addVec3F(
			renderBounding.maximum,
			renderPosition);

		bool isInFrustum = isBoxInFrustum3F(
			leftPlane,
			rightPlane,
			bottomPlane,
			topPlane,
			backPlane,
			frontPlane,
			renderBounding);

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
static void onRenderUpdate(void* argument)
{
	GraphicsRenderer renderer = argument;
	GraphicsRender* renders = renderer->renders;
	GraphicsRenderElement* renderElements =  renderer->renderElements;
	bool useCulling = renderer->useCulling;
	size_t renderCount = renderer->renderCount;
	const GraphicsRendererData* data = renderer->data;

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
	atomic_int64 threadIndex = atomicFetchAdd64(
		&renderer->threadIndex, 1);
	atomic_int64* elementIndex = &renderer->elementIndex;

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

		atomic_int64 index = atomicFetchAdd64(elementIndex, 1);
		renderElements[index] = element;
	}
}
GraphicsRendererResult drawGraphicsRenderer(
	GraphicsRenderer graphicsRenderer,
	const GraphicsRendererData* graphicsRendererData)
{
	assert(graphicsRenderer);
	assert(graphicsRendererData);
	assert(!graphicsRenderer->isEnumerating);

	GraphicsRendererResult result;
	result.renderCount = 0;
	result.indexCount = 0;
	result.passCount = 0;

	size_t renderCount = graphicsRenderer->renderCount;

	if (!renderCount)
		return result;

	Vec3F rendererPosition = negVec3F(getTranslationMat4F(
		graphicsRendererData->view));

	GraphicsRenderElement* renderElements = graphicsRenderer->renderElements;
	ThreadPool threadPool = graphicsRenderer->threadPool;

	size_t elementCount = 0;

	if (threadPool && renderCount > getThreadPoolThreadCount(threadPool))
	{
		waitThreadPool(threadPool);
		graphicsRenderer->data = graphicsRendererData;
		graphicsRenderer->threadIndex = 0;
		graphicsRenderer->elementIndex = 0;

		size_t threadCount = getThreadPoolThreadCount(threadPool);

		bool addResult = true;

		for (size_t i = 0; i < threadCount; i++)
		{
			addResult &= addThreadPoolTask(
				threadPool,
				onRenderUpdate,
				graphicsRenderer);
		}

		if (!addResult)
			abort();

		waitThreadPool(threadPool);
		elementCount = graphicsRenderer->elementIndex;
	}
	else
	{
		GraphicsRender* renders = graphicsRenderer->renders;
		bool useCulling = graphicsRenderer->useCulling;
		Plane3F leftPlane = graphicsRendererData->leftPlane;
		Plane3F rightPlane = graphicsRendererData->rightPlane;
		Plane3F bottomPlane = graphicsRendererData->bottomPlane;
		Plane3F topPlane = graphicsRendererData->topPlane;
		Plane3F backPlane = graphicsRendererData->backPlane;
		Plane3F frontPlane = graphicsRendererData->frontPlane;

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

	GraphicsRenderSorting sorting = graphicsRenderer->sorting;

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
		else
		{
			abort();
		}
	}

	Mat4F viewProj = graphicsRendererData->viewProj;
	GraphicsPipeline pipeline = graphicsRenderer->pipeline;
	OnGraphicsRenderDraw onDraw = graphicsRenderer->onDraw;

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
			result.renderCount++;
			result.indexCount += indexCount;
		}
	}

	return result;
}

GraphicsRender createGraphicsRender(
	GraphicsRenderer renderer,
	Transform transform,
	Box3F bounding,
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
	graphicsRender->bounding = bounding;

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
void destroyGraphicsRender(
	GraphicsRender graphicsRender,
	bool _destroyTransform)
{
	if (!graphicsRender)
		return;

	assert(!graphicsRender->renderer->isEnumerating);

	GraphicsRenderer renderer = graphicsRender->renderer;
	GraphicsRender* renders = renderer->renders;
	size_t renderCount = renderer->renderCount;

	for (size_t i = 0; i < renderCount; i++)
	{
		if (renders[i] != graphicsRender)
			continue;

		for (size_t j = i + 1; j < renderCount; j++)
			renders[j - 1] = renders[j];

		renderer->onDestroy(graphicsRender->handle);

		if (_destroyTransform)
			destroyTransform(graphicsRender->transform);

		free(graphicsRender);
		renderer->renderCount--;
		return;
	}

	abort();
}

GraphicsRenderer getGraphicsRenderRenderer(
	GraphicsRender graphicsRender)
{
	assert(graphicsRender);
	return graphicsRender->renderer;
}
Transform getGraphicsRenderTransform(
	GraphicsRender graphicsRender)
{
	assert(graphicsRender);
	return graphicsRender->transform;
}

Box3F getGraphicsRenderBounding(
	GraphicsRender graphicsRender)
{
	assert(graphicsRender);
	return graphicsRender->bounding;
}
void setGraphicsRenderBounding(
	GraphicsRender graphicsRender,
	Box3F bounding)
{
	assert(graphicsRender);
	graphicsRender->bounding = bounding;
}

void* getGraphicsRenderHandle(
	GraphicsRender graphicsRender)
{
	assert(graphicsRender);
	return graphicsRender->handle;
}
