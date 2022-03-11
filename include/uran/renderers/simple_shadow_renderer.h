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
#include "uran/pipelines/simple_shadow_pipeline.h"

/*
 * Create a new simple shadow renderer instance.
 * Returns simple shadow renderer instance in success, otherwise NULL.
 *
 * simpleShadowPipeline - simple shadow pipeline instance.
 * sorting - render sorting type.
 * useCulling - use frustum culling.
 * capacity - initial render array capacity or 0.
 * threadPool - thread pool instance or NULL.
 */
GraphicsRenderer createSimpleShadowRenderer(
	GraphicsPipeline simpleShadowPipeline,
	GraphicsRenderSorting sorting,
	bool useCulling,
	size_t capacity,
	ThreadPool threadPool);
/*
 * Create a new simple shadow render instance.
 * Returns simple shadow render instance on success, otherwise NULL.
 *
 * simpleShadowRenderer - simple shadow renderer instance.
 * transform - transform instance.
 * bounds - render bounds.
 * mesh - render mesh.
 */
GraphicsRender createSimpleShadowRender(
	GraphicsRenderer simpleShadowRenderer,
	Transform transform,
	Box3F bounds,
	GraphicsMesh mesh);

/*
 * Returns simple shadow render mesh.
 * simpleShadowRender - simple shadow render instance.
 */
GraphicsMesh getSimpleShadowRenderMesh(
	GraphicsRender simpleShadowRender);
/*
 * Sets simple shadow render mesh.
 *
 * simpleShadowRender - simple shadow render instance.
 * mesh - mesh instance.
 */
void setSimpleShadowRenderMesh(
	GraphicsRender simpleShadowRender,
	GraphicsMesh mesh);
