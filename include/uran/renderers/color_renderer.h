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
#include "mpgx/pipelines/color_pipeline.h"

GraphicsRenderer createColorRenderer(
	GraphicsPipeline colorPipeline,
	GraphicsRenderSorting sorting,
	bool useCulling,
	size_t capacity,
	ThreadPool threadPool);
GraphicsRender createColorRender(
	GraphicsRenderer colorRenderer,
	Transform transform,
	Box3F bounding,
	LinearColor color,
	GraphicsMesh mesh);

LinearColor getColorRenderColor(
	GraphicsRender colorRender);
void setColorRenderColor(
	GraphicsRender colorRender,
	LinearColor color);

GraphicsMesh getColorRenderMesh(
	GraphicsRender colorRender);
void setColorRenderMesh(
	GraphicsRender colorRender,
	GraphicsMesh mesh);
