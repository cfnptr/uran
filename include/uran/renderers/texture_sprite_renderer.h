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
#include "uran/pipelines/texture_sprite_pipeline.h"

/*
 * Create a new texture sprite renderer instance.
 * Returns texture sprite renderer instance in success, otherwise NULL.
 *
 * textureSpritePipeline - texture sprite pipeline instance.
 * sorting - render sorting type.
 * useCulling - use frustum culling.
 * capacity - initial render array capacity or 0.
 * threadPool - thread pool instance or NULL.
 */
GraphicsRenderer createTextureSpriteRenderer(
	GraphicsPipeline textureSpritePipeline,
	GraphicsRenderSorting sortingType,
	bool useCulling,
	size_t capacity,
	ThreadPool threadPool);
/*
 * Create a new texture sprite render instance.
 * Returns texture sprite render instance on success, otherwise NULL.
 *
 * textureSpriteRenderer - texture sprite renderer instance.
 * transform - transform instance.
 * bounds - render bounds.
 * color - render color.
 * size - texture size.
 * offset - texture offset.
 * mesh - render mesh.
 */
GraphicsRender createTextureSpriteRender(
	GraphicsRenderer textureSpriteRenderer,
	Transform transform,
	Box3F bounds,
	LinearColor color,
	Vec2F size,
	Vec2F offset,
	GraphicsMesh mesh);

/*
 * Returns texture sprite render color.
 * textureSpriteRender - texture sprite render instance.
 */
LinearColor getTextureSpriteRenderColor(
	GraphicsRender textureSpriteRender);
/*
 * Sets texture sprite render color.
 *
 * textureSpriteRender - texture sprite render instance.
 * color - color value.
 */
void setTextureSpriteRenderColor(
	GraphicsRender textureSpriteRender,
	LinearColor color);

/*
 * Returns texture sprite render size.
 * textureSpriteRender - texture sprite render instance.
 */
Vec2F getTextureSpriteRenderSize(
	GraphicsRender textureSpriteRender);
/*
 * Sets texture sprite render size.
 *
 * textureSpriteRender - texture sprite render instance.
 * size - texture size value.
 */
void setTextureSpriteRenderSize(
	GraphicsRender textureSpriteRender,
	Vec2F size);

/*
 * Returns texture sprite render offset.
 * textureSpriteRender - texture sprite render instance.
 */
Vec2F getTextureSpriteRenderOffset(
	GraphicsRender textureSpriteRender);
/*
 * Sets texture sprite render offset.
 *
 * textureSpriteRender - texture sprite render instance.
 * offset - texture offset value.
 */
void setTextureSpriteRenderOffset(
	GraphicsRender textureSpriteRender,
	Vec2F offset);

/*
 * Returns texture sprite render mesh.
 * textureSpriteRender - texture sprite render instance.
 */
GraphicsMesh getTextureSpriteRenderMesh(
	GraphicsRender textureSpriteRender);
/*
 * Sets texture sprite render mesh.
 *
 * textureSpriteRender - texture sprite render instance.
 * mesh - mesh instance.
 */
void setTextureSpriteRenderMesh(
	GraphicsRender textureSpriteRender,
	GraphicsMesh mesh);
