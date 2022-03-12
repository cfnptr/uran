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
#include "uran/pipelines/panel_pipeline.h"

/*
 * Create a new panel renderer instance.
 * Returns panel renderer instance in success, otherwise NULL.
 *
 * panelPipeline - panel pipeline instance.
 * sorting - render sorting type.
 * useCulling - use frustum culling.
 * capacity - initial render array capacity or 0.
 * threadPool - thread pool instance or NULL.
 */
GraphicsRenderer createPanelRenderer(
	GraphicsPipeline panelPipeline,
	GraphicsRenderSorting sorting,
	bool useCulling,
	size_t capacity,
	ThreadPool threadPool);
/*
 * Create a new panel render instance.
 * Returns panel render instance on success, otherwise NULL.
 *
 * panelRenderer - panel renderer instance.
 * transform - transform instance.
 * bounds - render bounds.
 * color - render color.
 * mesh - render mesh.
 */
GraphicsRender createPanelRender(
	GraphicsRenderer panelRenderer,
	Transform transform,
	Box3F bounds,
	LinearColor color);

/*
 * Returns panel render color.
 * panelRender - panel render instance.
 */
LinearColor getPanelRenderColor(
	GraphicsRender panelRender);
/*
 * Sets panel render color.
 *
 * panelRender - panel render instance.
 * color - color value.
 */
void setPanelRenderColor(
	GraphicsRender panelRender,
	LinearColor color);
