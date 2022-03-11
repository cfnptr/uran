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

#define DIFFUSE_PIPELINE_NAME "Diffuse"

/*
 * Create a new diffuse pipeline instance.
 * Returns operation MPGX result.
 *
 * framebuffer - framebuffer instance.
 * vertexShader - diffuse vertex shader.
 * fragmentShader - diffuse fragment shader.
 * state - pipeline state or NULL.
 * diffusePipeline - pointer to the diffuse pipeline.
 */
MpgxResult createDiffusePipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	const GraphicsPipelineState* state,
	GraphicsPipeline* diffusePipeline);

/*
 * Returns diffuse pipeline model view projection matrix.
 * diffusePipeline - diffuse pipeline instance.
 */
Mat4F getDiffusePipelineMvp(
	GraphicsPipeline diffusePipeline);
/*
 * Sets diffuse pipeline model view projection matrix.
 *
 * diffusePipeline - diffuse pipeline instance.
 * mvp - model view projection matrix value.
 */
void setDiffusePipelineMvp(
	GraphicsPipeline diffusePipeline,
	Mat4F mvp);

/*
 * Returns diffuse pipeline normal matrix.
 * diffusePipeline - diffuse pipeline instance.
 */
Mat4F getDiffusePipelineNormal(
	GraphicsPipeline diffusePipeline);
/*
 * Sets diffuse pipeline normal matrix.
 *
 * diffusePipeline - diffuse pipeline instance.
 * normal - normal matrix value.
 */
void setDiffusePipelineNormal(
	GraphicsPipeline diffusePipeline,
	Mat4F normal);

/*
 * Returns diffuse pipeline object color.
 * diffusePipeline - diffuse pipeline instance.
 */
LinearColor getDiffusePipelineObjectColor(
	GraphicsPipeline diffusePipeline);
/*
 * Sets diffuse pipeline object color.
 *
 * diffusePipeline - diffuse pipeline instance.
 * objectColor - object color value.
 */
void setDiffusePipelineObjectColor(
	GraphicsPipeline diffusePipeline,
	LinearColor objectColor);

/*
 * Returns diffuse pipeline ambient color.
 * diffusePipeline - diffuse pipeline instance.
 */
LinearColor getDiffusePipelineAmbientColor(
	GraphicsPipeline diffusePipeline);
/*
 * Sets diffuse pipeline ambient color.
 *
 * diffusePipeline - diffuse pipeline instance.
 * ambientColor - ambient color value.
 */
void setDiffusePipelineAmbientColor(
	GraphicsPipeline diffusePipeline,
	LinearColor ambientColor);

/*
 * Returns diffuse pipeline light color.
 * diffusePipeline - diffuse pipeline instance.
 */
LinearColor getDiffusePipelineLightColor(
	GraphicsPipeline diffusePipeline);
/*
 * Sets diffuse pipeline light color.
 *
 * diffusePipeline - diffuse pipeline instance.
 * lightColor - light color value.
 */
void setDiffusePipelineLightColor(
	GraphicsPipeline diffusePipeline,
	LinearColor lightColor);

/*
 * Returns diffuse pipeline light direction vector.
 * diffusePipeline - diffuse pipeline instance.
 */
Vec3F getDiffusePipelineLightDirection(
	GraphicsPipeline diffusePipeline);
/*
 * Sets diffuse pipeline light direction vector.
 *
 * diffusePipeline - diffuse pipeline instance.
 * lightColor - light direction vector value.
 */
void setDiffusePipelineLightDirection(
	GraphicsPipeline diffusePipeline,
	Vec3F lightDirection);
