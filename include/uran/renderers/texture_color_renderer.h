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
#include "uran/pipelines/texture_color_pipeline.h"

// TODO: possibly rename this to the just texture.
// texture_sprite to the sprite

/*
 * Create a new texture color renderer instance.
 * Returns texture color renderer instance in success, otherwise NULL.
 *
 * textureColorPipeline - texture color pipeline instance.
 * sorting - render sorting type.
 * useCulling - use frustum culling.
 * capacity - initial render array capacity or 0.
 * threadPool - thread pool instance or NULL.
 */
GraphicsRenderer createTextureColorRenderer(
	GraphicsPipeline textureColorPipeline,
	GraphicsRenderSorting sortingType,
	bool useCulling,
	size_t capacity,
	ThreadPool threadPool);
/*
 * Create a new texture color render instance.
 * Returns texture color render instance on success, otherwise NULL.
 *
 * textureColorRenderer - texture color renderer instance.
 * transform - transform instance.
 * bounds - render bounds.
 * color - render color.
 * size - texture size.
 * offset - texture offset.
 * mesh - render mesh.
 */
GraphicsRender createTextureColorRender(
	GraphicsRenderer textureColorRenderer,
	Transform transform,
	Box3F bounds,
	LinearColor color,
	Vec2F size,
	Vec2F offset,
	GraphicsMesh mesh);

/*
 * Returns texture color render color.
 * textureColorRender - texture color render instance.
 */
LinearColor getTextureColorRenderColor(
	GraphicsRender textureColorRender);
/*
 * Sets texture color render color.
 *
 * textureColorRender - texture color render instance.
 * color - color value.
 */
void setTextureColorRenderColor(
	GraphicsRender textureColorRender,
	LinearColor color);

/*
 * Returns texture color render size.
 * textureColorRender - texture color render instance.
 */
Vec2F getTextureColorRenderSize(
	GraphicsRender textureColorRender);
/*
 * Sets texture color render size.
 *
 * textureColorRender - texture color render instance.
 * size - texture size value.
 */
void setTextureColorRenderSize(
	GraphicsRender textureColorRender,
	Vec2F size);

/*
 * Returns texture color render offset.
 * textureColorRender - texture color render instance.
 */
Vec2F getTextureColorRenderOffset(
	GraphicsRender textureColorRender);
/*
 * Sets texture color render offset.
 *
 * textureColorRender - texture color render instance.
 * offset - texture offset value.
 */
void setTextureColorRenderOffset(
	GraphicsRender textureColorRender,
	Vec2F offset);

/*
 * Returns texture color render mesh.
 * textureColorRender - texture color render instance.
 */
GraphicsMesh getTextureColorRenderMesh(
	GraphicsRender textureColorRender);
/*
 * Sets texture color render mesh.
 *
 * textureColorRender - texture color render instance.
 * mesh - mesh instance.
 */
void setTextureColorRenderMesh(
	GraphicsRender textureColorRender,
	GraphicsMesh mesh);
