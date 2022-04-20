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
#include "uran/pipelines/diffuse_pipeline.h"

/*
 * Create a new diffuse renderer instance.
 * Returns diffuse renderer instance in success, otherwise NULL.
 *
 * diffusePipeline - diffuse pipeline instance.
 * sorting - render sorting type.
 * useCulling - use frustum culling.
 * capacity - initial render array capacity.
 * threadPool - thread pool instance or NULL.
 */
GraphicsRenderer createDiffuseRenderer(
	GraphicsPipeline diffusePipeline,
	GraphicsRenderSorting sorting,
	bool useCulling,
	size_t capacity,
	ThreadPool threadPool);
/*
 * Create a new diffuse render instance.
 * Returns diffuse render instance on success, otherwise NULL.
 *
 * diffuseRenderer - diffuse renderer instance.
 * transform - transform instance.
 * bounds - render bounds.
 * mesh - render mesh.
 */
GraphicsRender createDiffuseRender(
	GraphicsRenderer diffuseRenderer,
	Transform transform,
	Box3F bounds,
	GraphicsMesh mesh);

/*
 * Returns diffuse render mesh.
 * diffuseRender - diffuse render instance.
 */
GraphicsMesh getDiffuseRenderMesh(
	GraphicsRender diffuseRender);
/*
 * Sets diffuse render mesh.
 *
 * diffuseRender - diffuse render instance.
 * mesh - mesh instance.
 */
void setDiffuseRenderMesh(
	GraphicsRender diffuseRender,
	GraphicsMesh mesh);
