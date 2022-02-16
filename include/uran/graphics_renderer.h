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
#include "uran/text.h"

#include "mpgx/window.h"
#include "cmmt/camera.h"
#include "mpmt/thread_pool.h"

/*
 * Graphics renderer structure.
 */
typedef struct GraphicsRenderer_T GraphicsRenderer_T;
/*
 * Graphics renderer instance.
 */
typedef GraphicsRenderer_T* GraphicsRenderer;

/*
 * Graphics render structure.
 */
typedef struct GraphicsRender_T GraphicsRender_T;
/*
 * Graphics render instance.
 */
typedef GraphicsRender_T* GraphicsRender;

/*
 * Graphics render sorting types.
 */
typedef enum GraphicsRenderSorting_T
{
	NO_GRAPHICS_RENDER_SORTING = 0,
	ASCENDING_GRAPHICS_RENDER_SORTING = 1,
	DESCENDING_GRAPHICS_RENDER_SORTING = 2,
	GRAPHICS_RENDER_SORTING_COUNT = 3,
} RenderSorting_T;

/*
 * Graphics render sorting type.
 */
typedef uint8_t GraphicsRenderSorting;

/*
 * Graphics renderer data structure.
 */
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
/*
 * Graphics renderer data structure.
 */
typedef struct GraphicsRendererResult
{
	size_t renderCount;
	size_t indexCount;
	size_t passCount;
} GraphicsRendererResult;

/*
 * Graphics render destroy function.
 */
typedef void(*OnGraphicsRenderDestroy)(
	void* handle);
/*
 * Graphics render draw function.
 */
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
GraphicsRendererResult drawGraphicsRenderer(
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

inline static GraphicsRendererResult createGraphicsRendererResult()
{
	GraphicsRendererResult result;
	result.renderCount = 0;
	result.indexCount = 0;
	result.passCount = 0;
	return result;
}
inline static GraphicsRendererResult addGraphicsRendererResult(
	GraphicsRendererResult a, GraphicsRendererResult b)
{
	GraphicsRendererResult result;
	result.renderCount = a.renderCount + b.renderCount;
	result.indexCount = a.indexCount + b.indexCount;
	result.passCount = a.passCount + b.passCount;
	return result;
}
