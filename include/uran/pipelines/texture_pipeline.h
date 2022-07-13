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

#define TEXTURE_PIPELINE_NAME "Texture"

/*
 * Create a new texture pipeline instance.
 * Returns operation MPGX result.
 *
 * framebuffer - framebuffer instance.
 * vertexShader - texture vertex shader.
 * fragmentShader - texture fragment shader.
 * texture - texture instance.
 * sampler - texture sampler.
 * state - pipeline state or NULL.
 * texturePipeline - pointer to the texture pipeline.
 */
MpgxResult createTexturePipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	const GraphicsPipelineState* state,
	GraphicsPipeline* texturePipeline);

/*
 * Returns texture pipeline texture.
 * texturePipeline - texture pipeline instance.
 */
Image getTexturePipelineTexture(
	GraphicsPipeline texturePipeline);
/*
 * Returns texture pipeline sampler.
 * texturePipeline - texture pipeline instance.
 */
Sampler getTexturePipelineSampler(
	GraphicsPipeline texturePipeline);

/*
 * Returns texture pipeline model view projection matrix.
 * texturePipeline - texture pipeline instance.
 */
const mat4* getTexturePipelineMvp(
	GraphicsPipeline texturePipeline);
/*
 * Returns texture pipeline model view projection matrix.
 *
 * texturePipeline - texture pipeline instance.
 * mvp - model view projection matrix value.
 */
void setTexturePipelineMvp(
	GraphicsPipeline texturePipeline,
	const Mat4F* mvp);

/*
 * Returns texture pipeline size.
 * texturePipeline - texture pipeline instance.
 */
vec2 getTexturePipelineSize(
	GraphicsPipeline texturePipeline);
/*
 * Sets texture pipeline size.
 *
 * texturePipeline - texture pipeline instance.
 * size - texture size value.
 */
void setTexturePipelineSize(
	GraphicsPipeline texturePipeline,
	Vec2F size);

/*
 * Returns texture pipeline offset.
 * texturePipeline - texture pipeline instance.
 */
vec2 getTexturePipelineOffset(
	GraphicsPipeline texturePipeline);
/*
 * Sets texture pipeline offset.
 *
 * texturePipeline - texture pipeline instance.
 * offset - texture offset value.
 */
void setTexturePipelineOffset(
	GraphicsPipeline texturePipeline,
	Vec2F offset);

/*
 * Returns texture pipeline color.
 * texturePipeline - texture pipeline instance.
 */
vec4 getTexturePipelineColor(
	GraphicsPipeline texturePipeline);
/*
 * Sets texture pipeline color.
 *
 * texturePipeline - texture pipeline instance.
 * color - color value.
 */
void setTexturePipelineColor(
	GraphicsPipeline texturePipeline,
	LinearColor color);
