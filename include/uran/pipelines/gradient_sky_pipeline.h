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

#define GRADIENT_SKY_PIPELINE_NAME "GradientSky"

/*
 * Create a new gradient sky sampler instance.
 * Returns operation MPGX result.
 *
 * window - window instance.
 * gradientSkySampler - pointer to the gradient sky sampler.
 */
MpgxResult createGradientSkySampler(
	Window window,
	Sampler* gradientSkySampler);

/*
 * Create a new gradient sky pipeline instance.
 * Returns operation MPGX result.
 *
 * framebuffer - framebuffer instance.
 * vertexShader - gradient sky vertex shader.
 * fragmentShader - gradient sky fragment shader.
 * texture - sky texture.
 * sampler - texture sampler.
 * state - pipeline state or NULL.
 * gradientSkyPipeline - pointer to the gradient sky pipeline.
 */
MpgxResult createGradientSkyPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	const GraphicsPipelineState* state,
	GraphicsPipeline* gradientSkyPipeline);

/*
 * Returns gradient sky pipeline texture.
 * gradientSkyPipeline - gradient sky pipeline instance.
 */
Image getGradientSkyPipelineTexture(
	GraphicsPipeline gradientSkyPipeline);
/*
 * Returns gradient sky pipeline sampler.
 * gradientSkyPipeline - gradient sky pipeline instance.
 */
Sampler getGradientSkyPipelineSampler(
	GraphicsPipeline gradientSkyPipeline);

/*
 * Returns gradient sky pipeline model view projection matrix.
 * gradientSkyPipeline - gradient sky pipeline instance.
 */
Mat4F getGradientSkyPipelineMvp(
	GraphicsPipeline gradientSkyPipeline);
/*
 * Sets gradient sky pipeline model view projection matrix.
 *
 * gradientSkyPipeline - gradient sky pipeline instance.
 * mvp - model view projection matrix value.
 */
void setGradientSkyPipelineMvp(
	GraphicsPipeline gradientSkyPipeline,
	Mat4F mvp);

/*
 * Returns gradient sky pipeline sun direction vector.
 * gradientSkyPipeline - gradient sky pipeline instance.
 */
Vec3F getGradientSkyPipelineSunDirection(
	GraphicsPipeline gradientSkyPipeline);
/*
 * Sets gradient sky pipeline sun direction vector.
 *
 * gradientSkyPipeline - gradient sky pipeline instance.
 * sunDirection - sun direction vector value.
 */
void setGradientSkyPipelineSunDirection(
	GraphicsPipeline gradientSkyPipeline,
	Vec3F sunDirection);

/*
 * Returns gradient sky pipeline sun size.
 * gradientSkyPipeline - gradient sky pipeline instance.
 */
cmmt_float_t getGradientSkyPipelineSunSize(
	GraphicsPipeline gradientSkyPipeline);
/*
 * Sets gradient sky pipeline sun size.
 *
 * gradientSkyPipeline - gradient sky pipeline instance.
 * sunSize - sun size value.
 */
void setGradientSkyPipelineSunSize(
	GraphicsPipeline gradientSkyPipeline,
	cmmt_float_t sunSize);

/*
 * Returns gradient sky pipeline sun color.
 * gradientSkyPipeline - gradient sky pipeline instance.
 */
LinearColor getGradientSkyPipelineSunColor(
	GraphicsPipeline gradientSkyPipeline);
/*
 * Sets gradient sky pipeline sun color.
 *
 * gradientSkyPipeline - gradient sky pipeline instance.
 * sunColor - sun color value.
 */
void setGradientSkyPipelineSunColor(
	GraphicsPipeline gradientSkyPipeline,
	LinearColor sunColor);
