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

GraphicsRenderer createTextureSpriteRenderer(
	GraphicsPipeline textureSpritePipeline,
	GraphicsRenderSorting sortingType,
	bool useCulling,
	size_t capacity,
	ThreadPool threadPool);
GraphicsRender createTextureSpriteRender(
	GraphicsRenderer textureSpriteRenderer,
	Transform transform,
	Box3F bounding,
	LinearColor color,
	Vec2F size,
	Vec2F offset,
	GraphicsMesh mesh);

LinearColor getTextureSpriteRenderColor(
	GraphicsRender textureSpriteRender);
void setTextureSpriteRenderColor(
	GraphicsRender textureSpriteRender,
	LinearColor color);

Vec2F getTextureSpriteRenderSize(
	GraphicsRender textureSpriteRender);
void setTextureSpriteRenderSize(
	GraphicsRender textureSpriteRender,
	Vec2F size);

Vec2F getTextureSpriteRenderOffset(
	GraphicsRender textureSpriteRender);
void setTextureSpriteRenderOffset(
	GraphicsRender textureSpriteRender,
	Vec2F offset);

GraphicsMesh getTextureSpriteRenderMesh(
	GraphicsRender textureSpriteRender);
void setTextureSpriteRenderMesh(
	GraphicsRender textureSpriteRender,
	GraphicsMesh mesh);
