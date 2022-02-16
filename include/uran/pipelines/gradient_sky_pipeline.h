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

MpgxResult createGradientSkySampler(
	Window window,
	Sampler* gradientSkySampler);

MpgxResult createGradientSkyPipelineExt(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	const GraphicsPipelineState* state,
	GraphicsPipeline* gradientSky);
MpgxResult createGradientSkyPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	GraphicsPipeline* gradientSky);

Image getGradientSkyPipelineTexture(
	GraphicsPipeline gradientSkyPipeline);
Sampler getGradientSkyPipelineSampler(
	GraphicsPipeline gradientSkyPipeline);

Mat4F getGradientSkyPipelineMvp(
	GraphicsPipeline gradientSkyPipeline);
void setGradientSkyPipelineMvp(
	GraphicsPipeline gradientSkyPipeline,
	Mat4F mvp);

Vec3F getGradientSkyPipelineSunDir(
	GraphicsPipeline gradientSkyPipeline);
void setGradientSkyPipelineSunDir(
	GraphicsPipeline gradientSkyPipeline,
	Vec3F sunDir);

LinearColor getGradientSkyPipelineSunColor(
	GraphicsPipeline gradientSkyPipeline);
void setGradientSkyPipelineSunColor(
	GraphicsPipeline gradientSkyPipeline,
	LinearColor sunColor);
