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
#include "uran/pipelines/color_pipeline.h"

/*
 * Create a new color renderer instance.
 * Returns color renderer instance in success, otherwise NULL.
 *
 * colorPipeline - color pipeline instance.
 * sorting - render sorting type.
 * useCulling - use frustum culling.
 * capacity - initial render array capacity.
 * threadPool - thread pool instance or NULL.
 */
GraphicsRenderer createColorRenderer(
	GraphicsPipeline colorPipeline,
	GraphicsRenderSorting sorting,
	bool useCulling,
	size_t capacity,
	ThreadPool threadPool);
/*
 * Create a new color render instance.
 * Returns color render instance on success, otherwise NULL.
 *
 * colorRenderer - color renderer instance.
 * transform - transform instance.
 * bounds - render bounds.
 * color - render color.
 * mesh - render mesh.
 */
GraphicsRender createColorRender(
	GraphicsRenderer colorRenderer,
	Transform transform,
	Box3F bounds,
	LinearColor color,
	GraphicsMesh mesh);

/*
 * Returns color render color.
 * colorRender - color render instance.
 */
LinearColor getColorRenderColor(
	GraphicsRender colorRender);
/*
 * Sets color render color.
 *
 * colorRender - color render instance.
 * color - color value.
 */
void setColorRenderColor(
	GraphicsRender colorRender,
	LinearColor color);

/*
 * Returns color render mesh.
 * colorRender - color render instance.
 */
GraphicsMesh getColorRenderMesh(
	GraphicsRender colorRender);
/*
 * Sets color render mesh.
 *
 * colorRender - color render instance.
 * mesh - mesh instance.
 */
void setColorRenderMesh(
	GraphicsRender colorRender,
	GraphicsMesh mesh);
