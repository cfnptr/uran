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

#define GAUSSIAN_BLUR_PIPELINE_NAME "GaussianBlur"

/*
 * Create a new gaussian blur pipeline instance.
 * Returns operation MPGX result.
 *
 * framebuffer - framebuffer instance.
 * vertexShader - gaussian blur vertex shader.
 * fragmentShader - gaussian blur fragment shader.
 * buffer - buffer image.
 * sampler - image sampler.
 * state - pipeline state or NULL.
 * gaussianBlurPipeline - pointer to the gaussian blur pipeline.
 */
MpgxResult createGaussianBlurPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image buffer,
	Sampler sampler,
	const GraphicsPipelineState* state,
	GraphicsPipeline* gaussianBlurPipeline);

/*
 * Returns gaussian blur pipeline buffer.
 * gaussianBlurPipeline - gaussian blur pipeline instance.
 */
Image getGaussianBlurPipelineBuffer(
	GraphicsPipeline gaussianBlurPipeline);
/*
 * Returns gaussian blur pipeline sampler.
 * gaussianBlurPipeline - gaussian blur pipeline instance.
 */
Sampler getGaussianBlurPipelineSampler(
	GraphicsPipeline gaussianBlurPipeline);

/*
 * Returns gaussian blur pipeline radius.
 * gaussianBlurPipeline - gaussian blur pipeline instance.
 */
int getGaussianBlurPipelineRadius(
	GraphicsPipeline gaussianBlurPipeline);
/*
 * Sets gaussian blur pipeline radius.
 *
 * gaussianBlurPipeline - gaussian blur pipeline instance.
 * radius - blur radius value.
 */
void setGaussianBlurPipelineRadius(
	GraphicsPipeline gaussianBlurPipeline,
	int radius);
