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
#include "uran/graphics_renderer.h"
#include "uran/pipelines/texture_pipeline.h"

/*
 * Create a new texture renderer instance.
 * Returns texture renderer instance in success, otherwise NULL.
 *
 * texturePipeline - texture pipeline instance.
 * sorting - render sorting type.
 * useCulling - use frustum culling.
 * capacity - initial render array capacity or 0.
 * threadPool - thread pool instance or NULL.
 */
GraphicsRenderer createTextureRenderer(
	GraphicsPipeline texturePipeline,
	GraphicsRenderSorting sortingType,
	bool useCulling,
	size_t capacity,
	ThreadPool threadPool);
/*
 * Create a new texture render instance.
 * Returns texture render instance on success, otherwise NULL.
 *
 * textureRenderer - texture renderer instance.
 * transform - transform instance.
 * bounds - render bounds.
 * color - render color.
 * size - texture size.
 * offset - texture offset.
 * mesh - render mesh.
 */
GraphicsRender createTextureRender(
	GraphicsRenderer textureRenderer,
	Transform transform,
	Box3F bounds,
	LinearColor color,
	Vec2F size,
	Vec2F offset,
	GraphicsMesh mesh);

/*
 * Returns texture render color.
 * textureRender - texture render instance.
 */
LinearColor getTextureRenderColor(
	GraphicsRender textureRender);
/*
 * Sets texture render color.
 *
 * textureRender - texture render instance.
 * color - color value.
 */
void setTextureRenderColor(
	GraphicsRender textureRender,
	LinearColor color);

/*
 * Returns texture render size.
 * textureRender - texture render instance.
 */
Vec2F getTextureRenderSize(
	GraphicsRender textureRender);
/*
 * Sets texture render size.
 *
 * textureRender - texture render instance.
 * size - texture size value.
 */
void setTextureRenderSize(
	GraphicsRender textureRender,
	Vec2F size);

/*
 * Returns texture render offset.
 * textureRender - texture render instance.
 */
Vec2F getTextureRenderOffset(
	GraphicsRender textureRender);
/*
 * Sets texture render offset.
 *
 * textureRender - texture render instance.
 * offset - texture offset value.
 */
void setTextureRenderOffset(
	GraphicsRender textureRender,
	Vec2F offset);

/*
 * Returns texture render mesh.
 * textureRender - texture render instance.
 */
GraphicsMesh getTextureRenderMesh(
	GraphicsRender textureRender);
/*
 * Sets texture render mesh.
 *
 * textureRender - texture render instance.
 * mesh - mesh instance.
 */
void setTextureRenderMesh(
	GraphicsRender textureRender,
	GraphicsMesh mesh);
