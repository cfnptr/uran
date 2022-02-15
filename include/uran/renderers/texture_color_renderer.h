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
#include "mpgx/pipelines/texture_color_pipeline.h"

GraphicsRenderer createTextureColorRenderer(
	GraphicsPipeline textureColorPipeline,
	GraphicsRenderSorting sortingType,
	bool useCulling,
	size_t capacity,
	ThreadPool threadPool);
GraphicsRender createTextureColorRender(
	GraphicsRenderer textureColorRenderer,
	Transform transform,
	Box3F bounding,
	LinearColor color,
	Vec2F size,
	Vec2F offset,
	GraphicsMesh mesh);

LinearColor getTextureColorRenderColor(
	GraphicsRender textureColorRender);
void setTextureColorRenderColor(
	GraphicsRender textureColorRender,
	LinearColor color);

Vec2F getTextureColorRenderSize(
	GraphicsRender textureColorRender);
void setTextureColorRenderSize(
	GraphicsRender textureColorRender,
	Vec2F size);

Vec2F getTextureColorRenderOffset(
	GraphicsRender textureColorRender);
void setTextureColorRenderOffset(
	GraphicsRender textureColorRender,
	Vec2F offset);

GraphicsMesh getTextureColorRenderMesh(
	GraphicsRender textureColorRender);
void setTextureColorRenderMesh(
	GraphicsRender textureColorRender,
	GraphicsMesh mesh);
