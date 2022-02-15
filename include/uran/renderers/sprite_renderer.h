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
#include "mpgx/pipelines/sprite_pipeline.h"

GraphicsRenderer createSpriteRenderer(
	GraphicsPipeline spritePipeline,
	GraphicsRenderSorting sorting,
	bool useCulling,
	size_t capacity,
	ThreadPool threadPool);
GraphicsRender createSpriteRender(
	GraphicsRenderer spriteRenderer,
	Transform transform,
	Box3F bounding,
	LinearColor color,
	GraphicsMesh mesh);

LinearColor getSpriteRenderColor(
	GraphicsRender spriteRender);
void setSpriteRenderColor(
	GraphicsRender spriteRender,
	LinearColor color);

GraphicsMesh getSpriteRenderMesh(
	GraphicsRender spriteRender);
void setSpriteRenderMesh(
	GraphicsRender spriteRender,
	GraphicsMesh mesh);
