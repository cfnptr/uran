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
#include "cmmt/color.h"

#define BLOOM_PIPELINE_NAME "Bloom"

/*
 * Create a new bloom pipeline instance.
 * Returns operation MPGX result.
 *
 * framebuffer - framebuffer instance.
 * vertexShader - bloom vertex shader.
 * fragmentShader - bloom fragment shader.
 * buffer - buffer image.
 * sampler - image sampler.
 * state - pipeline state or NULL.
 * bloomPipeline - pointer to the bloom pipeline.
 */
MpgxResult createBloomPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image buffer,
	Sampler sampler,
	const GraphicsPipelineState* state,
	GraphicsPipeline* bloomPipeline);

/*
 * Returns bloom pipeline buffer.
 * bloomPipeline - bloom pipeline instance.
 */
Image getBloomPipelineBuffer(
	GraphicsPipeline bloomPipeline);
/*
 * Returns bloom pipeline sampler.
 * bloomPipeline - bloom pipeline instance.
 */
Sampler getBloomPipelineSampler(
	GraphicsPipeline bloomPipeline);

/*
 * Returns bloom pipeline threshold.
 * bloomPipeline - bloom pipeline instance.
 */
LinearColor getBloomPipelineThreshold(
	GraphicsPipeline bloomPipeline);
/*
 * Returns bloom pipeline threshold.
 *
 * bloomPipeline - bloom pipeline instance.
 * threshold - color threshold value.
 */
void setBloomPipelineThreshold(
	GraphicsPipeline bloomPipeline,
	LinearColor threshold);
