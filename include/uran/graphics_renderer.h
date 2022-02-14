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
	UI_ASCENDING_GRAPHICS_RENDER_SORTING = 3,
	UI_DESCENDING_GRAPHICS_RENDER_SORTING = 4,
	GRAPHICS_RENDER_SORTING_COUNT = 5,
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
	size_t drawCount;
	size_t indexCount;
	size_t passCount;
} GraphicsRendererResult;

/*
 * Graphics render destroy function.
 * handle - handle instance or NULL.
 */
typedef void(*OnGraphicsRenderDestroy)(
	void* handle);
/*
 * Graphics render draw function.
 * Returns rendered index count.
 *
 * graphicsRender - graphics render instance.
 * graphicsPipeline - graphics pipeline instance.
 * model - pointer to the model matrix.
 * viewProj - pointer to the view projection matrix.
 */
typedef size_t(*OnGraphicsRenderDraw)(
	GraphicsRender graphicsRender,
	GraphicsPipeline graphicsPipeline,
	const Mat4F* model,
	const Mat4F* viewProj);
/*
 * Graphics renderer enumeration function.
 *
 * graphicsRender - graphics render instance.
 * handle - handle instance or NULL.
 */
typedef void(*OnGraphicsRendererItem)(
	GraphicsRender graphicsRender, void* handle);

/*
 * Create a new graphics renderer instance.
 * Returns graphics renderer instance on success, otherwise NULL.
 *
 * pipeline - graphics pipeline instance.
 * sorting - graphics render sorting type.
 * useCulling - use frustum culling.
 * onDestroy - on graphics render destroy function.
 * onDraw - on graphics render draw function.
 * capacity - initial render array capacity .
 * threadPool - thread pool instance or NULL.
 */
GraphicsRenderer createGraphicsRenderer(
	GraphicsPipeline pipeline,
	GraphicsRenderSorting sorting,
	bool useCulling,
	OnGraphicsRenderDestroy onDestroy,
	OnGraphicsRenderDraw onDraw,
	size_t capacity,
	ThreadPool threadPool);
/*
 * Destroys graphics renderer instance.
 * renderer - graphics renderer instance or NULL.
 */
void destroyGraphicsRenderer(GraphicsRenderer renderer);

/*
 * Returns graphics renderer pipeline instance.
 * renderer - graphics renderer instance.
 */
GraphicsPipeline getGraphicsRendererPipeline(GraphicsRenderer renderer);
/*
 * Returns graphics renderer on render destroy function.
 * renderer - graphics renderer instance.
 */
OnGraphicsRenderDestroy getGraphicsRendererOnDestroy(GraphicsRenderer renderer);
/*
 * Returns graphics renderer on render draw function.
 * renderer - graphics renderer instance.
 */
OnGraphicsRenderDraw getGraphicsRendererOnDraw(GraphicsRenderer renderer);
/*
 * Returns graphics renderer thread pool instance.
 * renderer - graphics renderer instance.
 */
ThreadPool getGraphicsRendererThreadPool(GraphicsRenderer renderer);
/*
 * Returns graphics renderer render count.
 * renderer - graphics renderer instance.
 */
size_t getGraphicsRendererRenderCount(GraphicsRenderer renderer);

/*
 * Returns graphics renderer sorting type.
 * renderer - graphics renderer instance.
 */
GraphicsRenderSorting getGraphicsRendererSorting(
	GraphicsRenderer renderer);
/*
 * Sets graphics renderer sorting type.
 * renderer - graphics renderer instance.
 */
void setGraphicsRendererSorting(
	GraphicsRenderer renderer,
	GraphicsRenderSorting sorting);

/*
 * Returns graphics renderer use frustum culling.
 * renderer - graphics renderer instance.
 */
bool getGraphicsRendererUseCulling(
	GraphicsRenderer renderer);
/*
 * Sets graphics renderer use frustum culling.
 * renderer - graphics renderer instance.
 */
void setGraphicsRendererUseCulling(
	GraphicsRenderer renderer,
	bool useCulling);

/*
 * Enumerates graphics renderer renders.
 *
 * renderer - graphics renderer instance.
 * onItem - on graphics renderer item function.
 * handle - function argument or NULL.
 */
void enumerateGraphicsRendererItems(
	GraphicsRenderer renderer,
	OnGraphicsRendererItem onItem,
	void* handle);
/*
 * Destroys all graphics renderer renders.
 * renderer - graphics renderer instance.
 */
void destroyAllGraphicsRendererItems(
	GraphicsRenderer renderer,
	bool destroyTransforms);

/*
 * Draws graphics renderer renders.
 *
 * renderer - graphics renderer instance.
 * data - graphics renderer data.
 */
GraphicsRendererResult drawGraphicsRenderer(
	GraphicsRenderer renderer,
	const GraphicsRendererData* data);

/*
 * Create a new graphics render instance.
 * Returns graphics render instance on success, otherwise NULL.
 *
 * renderer - graphics renderer instance.
 * transform - transform instance.
 * bounds - graphics render bounds.
 * handle - graphics render handle.
 */
GraphicsRender createGraphicsRender(
	GraphicsRenderer renderer,
	Transform transform,
	Box3F bounds,
	void* handle);
/*
 * Destroys graphics render instance.
 * render - graphics render instance or NULL.
 */
void destroyGraphicsRender(GraphicsRender render);

/*
 * Returns graphics render handle.
 * render - graphics render instance.
 */
void* getGraphicsRenderHandle(GraphicsRender render);
/*
 * Returns graphics render renderer instance.
 * render - graphics render instance.
 */
GraphicsRenderer getGraphicsRenderRenderer(GraphicsRender render);
/*
 * Returns graphics render transform instance.
 * render - graphics render instance.
 */
Transform getGraphicsRenderTransform(GraphicsRender render);

/*
 * Returns graphics render bounds.
 * render - graphics render instance.
 */
Box3F getGraphicsRenderBounds(
	GraphicsRender render);
/*
 * Sets graphics render bounds.
 *
 * render - graphics render instance.
 * bounds - bounds value.
 */
void setGraphicsRenderBounds(
	GraphicsRender render,
	Box3F bounds);

/*
 * Creates graphics renderer data.
 *
 * view - camera view matrix.
 * camera - camera value.
 * createPlanes - create planes for frustum culling.
 */
inline static GraphicsRendererData createGraphicsRenderData(
	Mat4F view,
	Camera camera,
	bool createPlanes)
{
	GraphicsRendererData data;
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
			viewProj = dotMat4F(proj, view);

			if (createPlanes)
			{
				frustumZeroOneMat4F(
					viewProj,
					&data.leftPlane,
					&data.rightPlane,
					&data.bottomPlane,
					&data.topPlane,
					&data.backPlane,
					&data.frontPlane,
					false);
			}
			else
			{
				data.leftPlane = plane3F(zeroVec3F, 0.0f);
				data.rightPlane = plane3F(zeroVec3F, 0.0f);
				data.bottomPlane = plane3F(zeroVec3F, 0.0f);
				data.topPlane = plane3F(zeroVec3F, 0.0f);
				data.backPlane = plane3F(zeroVec3F, 0.0f);
				data.frontPlane = plane3F(zeroVec3F, 0.0f);
			}
		}
		else if (api == OPENGL_GRAPHICS_API)
		{
			proj = perspNegOneMat4F(
				camera.persp.fieldOfView,
				camera.persp.aspectRatio,
				camera.persp.nearClipPlane,
				camera.persp.farClipPlane);
			viewProj = dotMat4F(proj, view);

			if (createPlanes)
			{
				frustumNegOneMat4F(
					viewProj,
					&data.leftPlane,
					&data.rightPlane,
					&data.bottomPlane,
					&data.topPlane,
					&data.backPlane,
					&data.frontPlane,
					false);
			}
			else
			{
				data.leftPlane = plane3F(zeroVec3F, 0.0f);
				data.rightPlane = plane3F(zeroVec3F, 0.0f);
				data.bottomPlane = plane3F(zeroVec3F, 0.0f);
				data.topPlane = plane3F(zeroVec3F, 0.0f);
				data.backPlane = plane3F(zeroVec3F, 0.0f);
				data.frontPlane = plane3F(zeroVec3F, 0.0f);
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
			viewProj = dotMat4F(proj, view);

			if (createPlanes)
			{
				frustumZeroOneMat4F(
					viewProj,
					&data.leftPlane,
					&data.rightPlane,
					&data.bottomPlane,
					&data.topPlane,
					&data.backPlane,
					&data.frontPlane,
					false);
			}
			else
			{
				data.leftPlane = plane3F(zeroVec3F, 0.0f);
				data.rightPlane = plane3F(zeroVec3F, 0.0f);
				data.bottomPlane = plane3F(zeroVec3F, 0.0f);
				data.topPlane = plane3F(zeroVec3F, 0.0f);
				data.backPlane = plane3F(zeroVec3F, 0.0f);
				data.frontPlane = plane3F(zeroVec3F, 0.0f);
			}
		}
		else if (api == OPENGL_GRAPHICS_API)
		{
			proj = orthoNegOneMat4F(
				camera.ortho.leftFrustum,
				camera.ortho.rightFrustum,
				camera.ortho.bottomFrustum,
				camera.ortho.topFrustum,
				camera.ortho.nearClipPlane,
				camera.ortho.farClipPlane);
			viewProj = dotMat4F(proj, view);

			if (createPlanes)
			{
				frustumNegOneMat4F(
					viewProj,
					&data.leftPlane,
					&data.rightPlane,
					&data.bottomPlane,
					&data.topPlane,
					&data.backPlane,
					&data.frontPlane,
					false);
			}
			else
			{
				data.leftPlane = plane3F(zeroVec3F, 0.0f);
				data.rightPlane = plane3F(zeroVec3F, 0.0f);
				data.bottomPlane = plane3F(zeroVec3F, 0.0f);
				data.topPlane = plane3F(zeroVec3F, 0.0f);
				data.backPlane = plane3F(zeroVec3F, 0.0f);
				data.frontPlane = plane3F(zeroVec3F, 0.0f);
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

	data.view = view;
	data.proj = proj;
	data.viewProj = viewProj;
	return data;
}

/*
 * Creates graphics renderer result.
 */
inline static GraphicsRendererResult createGraphicsRendererResult()
{
	GraphicsRendererResult result;
	result.drawCount = 0;
	result.indexCount = 0;
	result.passCount = 0;
	return result;
}
/*
 * Adds graphics renderer result.
 *
 * a - first graphics renderer result.
 * b - second graphics renderer result.
 */
inline static GraphicsRendererResult addGraphicsRendererResult(
	GraphicsRendererResult a, GraphicsRendererResult b)
{
	GraphicsRendererResult result;
	result.drawCount = a.drawCount + b.drawCount;
	result.indexCount = a.indexCount + b.indexCount;
	result.passCount = a.passCount + b.passCount;
	return result;
}
