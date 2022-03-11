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
#include "uran/pipelines/sprite_pipeline.h"

/*
 * Create a new sprite renderer instance.
 * Returns sprite renderer instance in success, otherwise NULL.
 *
 * spritePipeline - sprite pipeline instance.
 * sorting - render sorting type.
 * useCulling - use frustum culling.
 * capacity - initial render array capacity or 0.
 * threadPool - thread pool instance or NULL.
 */
GraphicsRenderer createSpriteRenderer(
	GraphicsPipeline spritePipeline,
	GraphicsRenderSorting sorting,
	bool useCulling,
	size_t capacity,
	ThreadPool threadPool);
/*
 * Create a new sprite render instance.
 * Returns sprite render instance on success, otherwise NULL.
 *
 * spriteRenderer - sprite renderer instance.
 * transform - transform instance.
 * bounds - render bounds.
 * color - render color.
 * mesh - render mesh.
 */
GraphicsRender createSpriteRender(
	GraphicsRenderer spriteRenderer,
	Transform transform,
	Box3F bounds,
	LinearColor color,
	GraphicsMesh mesh);

/*
 * Returns sprite render color.
 * spriteRender - sprite render instance.
 */
LinearColor getSpriteRenderColor(
	GraphicsRender spriteRender);
/*
 * Sets sprite render color.
 *
 * spriteRender - sprite render instance.
 * color - color value.
 */
void setSpriteRenderColor(
	GraphicsRender spriteRender,
	LinearColor color);

/*
 * Returns sprite render mesh.
 * spriteRender - sprite render instance.
 */
GraphicsMesh getSpriteRenderMesh(
	GraphicsRender spriteRender);
/*
 * Sets sprite render mesh.
 *
 * spriteRender - sprite render instance.
 * mesh - mesh instance.
 */
void setSpriteRenderMesh(
	GraphicsRender spriteRender,
	GraphicsMesh mesh);
