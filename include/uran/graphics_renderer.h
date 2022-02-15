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

#pragma once
#include "uran/transformer.h"

#include "mpgx/text.h"
#include "mpgx/window.h"
#include "cmmt/camera.h"
#include "mpmt/thread_pool.h"

typedef struct GraphicsRenderer_T GraphicsRenderer_T;
typedef GraphicsRenderer_T* GraphicsRenderer;

typedef struct GraphicsRender_T GraphicsRender_T;
typedef GraphicsRender_T* GraphicsRender;

typedef enum GraphicsRenderSorting_T
{
	NO_GRAPHICS_RENDER_SORTING = 0,
	ASCENDING_GRAPHICS_RENDER_SORTING = 1,
	DESCENDING_GRAPHICS_RENDER_SORTING = 2,
	GRAPHICS_RENDER_SORTING_COUNT = 3,
} RenderSorting_T;

typedef uint8_t GraphicsRenderSorting;

typedef struct GraphicsRendererData
{
	Mat4F view;
	Mat4F proj;
	Mat4F viewProj;
	Plane3F leftPlane;
	Plane3F rightPlane;
	Plane3F bottomPlane;
	Plane3F topPlane;
	Plane3F backPlane;
	Plane3F frontPlane;
} GraphicsRendererData;
typedef struct GraphicsRenderResult
{
	size_t renderCount;
	size_t indexCount;
	size_t passCount;
} GraphicsRenderResult;

typedef void(*OnGraphicsRenderDestroy)(
	void* handle);
typedef size_t(*OnGraphicsRenderDraw)(
	GraphicsRender graphicsRender,
	GraphicsPipeline graphicsPipeline,
	const Mat4F* model,
	const Mat4F* viewProj);

GraphicsRenderer createGraphicsRenderer(
	GraphicsPipeline pipeline,
	GraphicsRenderSorting sorting,
	bool useCulling,
	OnGraphicsRenderDestroy onDestroy,
	OnGraphicsRenderDraw onDraw,
	size_t capacity,
	ThreadPool threadPool);
void destroyGraphicsRenderer(
	GraphicsRenderer graphicsRenderer);

GraphicsPipeline getGraphicsRendererPipeline(
	GraphicsRenderer graphicsRenderer);
OnGraphicsRenderDestroy getGraphicsRendererOnDestroy(
	GraphicsRenderer graphicsRenderer);
OnGraphicsRenderDraw getGraphicsRendererOnDraw(
	GraphicsRenderer graphicsRenderer);
ThreadPool getGraphicsRendererThreadPool(
	GraphicsRenderer graphicsRenderer);

GraphicsRenderSorting getGraphicsRendererSorting(
	GraphicsRenderer graphicsRenderer);
void setGraphicsRendererSorting(
	GraphicsRenderer graphicsRenderer,
	GraphicsRenderSorting sorting);

bool getGraphicsRendererUseCulling(
	GraphicsRenderer renderer);
void setGraphicsRendererUseCulling(
	GraphicsRenderer renderer,
	bool useCulling);

size_t getGraphicsRendererRenderCount(
	GraphicsRenderer graphicsRenderer);
void enumerateGraphicsRenderer(
	GraphicsRenderer graphicsRenderer,
	void(*onItem)(GraphicsRender));
void destroyAllGraphicsRendererRenders(
	GraphicsRenderer graphicsRenderer,
	bool destroyTransforms);

void createGraphicsRenderData(
	Mat4F view,
	Camera camera,
	GraphicsRendererData* graphicsRendererData,
	bool createPlanes);
GraphicsRenderResult drawGraphicsRenderer(
	GraphicsRenderer graphicsRenderer,
	const GraphicsRendererData* graphicsRendererData);

GraphicsRender createGraphicsRender(
	GraphicsRenderer renderer,
	Transform transform,
	Box3F bounding,
	void* handle);
void destroyGraphicsRender(
	GraphicsRender graphicsRender,
	bool destroyTransform);

GraphicsRenderer getGraphicsRenderRenderer(
	GraphicsRender graphicsRender);
Transform getGraphicsRenderTransform(
	GraphicsRender graphicsRender);

Box3F getGraphicsRenderBounding(
	GraphicsRender graphicsRender);
void setGraphicsRenderBounding(
	GraphicsRender graphicsRender,
	Box3F bounding);

void* getGraphicsRenderHandle(
	GraphicsRender graphicsRender);

inline static GraphicsRenderResult createGraphicsRenderResult()
{
	GraphicsRenderResult result;
	result.renderCount = 0;
	result.indexCount = 0;
	result.passCount = 0;
	return result;
}
inline static GraphicsRenderResult addGraphicsRenderResult(
	GraphicsRenderResult a, GraphicsRenderResult b)
{
	GraphicsRenderResult result;
	result.renderCount = a.renderCount + b.renderCount;
	result.indexCount = a.indexCount + b.indexCount;
	result.passCount = a.passCount + b.passCount;
	return result;
}
