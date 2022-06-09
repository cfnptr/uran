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
#include "mpgx/window.h"

#define SIMPLE_SHADOW_PIPELINE_NAME "SimpleShadow"

/*
 * Create a new simple shadow sampler instance.
 *
 * window - window instance.
 * simpleShadowSampler - pointer to the simple shadow sampler.
 */
MpgxResult createSimpleShadowSampler(
	Window window,
	Sampler* simpleShadowSampler);

/*
 * Create a new simple shadow pipeline instance.
 * Returns operation MPGX result.
 *
 * framebuffer - framebuffer instance.
 * vertexShader - simple shadow vertex shader.
 * fragmentShader - simple shadow fragment shader.
 * state - pipeline state or NULL.
 * simpleShadowPipeline - pointer to the simple shadow pipeline.
 */
MpgxResult createSimpleShadowPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	const GraphicsPipelineState* state,
	GraphicsPipeline* simpleShadowPipeline);

/*
 * Returns simple shadow pipeline model view projection matrix.
 * simpleShadowPipeline - simple shadow pipeline instance.
 */
Mat4F getSimpleShadowPipelineMvp(
	GraphicsPipeline simpleShadowPipeline);
/*
 * Sets simple shadow pipeline model view projection matrix.
 *
 * simpleShadowPipeline - simple shadow pipeline instance.
 * mvp - model view projection matrix value.
 */
void setSimpleShadowPipelineMvp(
	GraphicsPipeline simpleShadowPipeline,
	Mat4F mvp);
