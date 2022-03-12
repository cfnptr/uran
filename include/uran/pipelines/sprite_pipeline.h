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

#define SPRITE_PIPELINE_NAME "Sprite"

/*
 * Create a new sprite pipeline instance.
 * Returns operation MPGX result.
 *
 * framebuffer - framebuffer instance.
 * vertexShader - sprite vertex shader.
 * fragmentShader - sprite fragment shader.
 * texture - texture instance.
 * sampler - texture sampler.
 * state - pipeline state or NULL.
 * spritePipeline - pointer to the sprite pipeline.
 */
MpgxResult createSpritePipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	const GraphicsPipelineState* state,
	GraphicsPipeline* spritePipeline);

/*
 * Returns sprite pipeline texture.
 * spritePipeline - sprite pipeline instance.
 */
Image getSpritePipelineTexture(
	GraphicsPipeline spritePipeline);
/*
 * Returns sprite pipeline sampler.
 * spritePipeline - sprite pipeline instance.
 */
Sampler getSpritePipelineSampler(
	GraphicsPipeline spritePipeline);

/*
 * Returns sprite pipeline model view projection matrix.
 * spritePipeline - sprite pipeline instance.
 */
Mat4F getSpritePipelineMvp(
	GraphicsPipeline spritePipeline);
/*
 * Sets sprite pipeline model view projection matrix.
 *
 * spritePipeline - sprite pipeline instance.
 * mvp - model view projection matrix.
 */
void setSpritePipelineMvp(
	GraphicsPipeline spritePipeline,
	Mat4F mvp);

/*
 * Returns sprite pipeline size.
 * spritePipeline - sprite pipeline instance.
 */
Vec2F getSpritePipelineSize(
	GraphicsPipeline spritePipeline);
/*
 * Sets sprite pipeline size.
 *
 * spritePipeline - sprite pipeline instance.
 * size - texture size value.
 */
void setSpritePipelineSize(
	GraphicsPipeline spritePipeline,
	Vec2F size);

/*
 * Returns sprite pipeline offset.
 * spritePipeline - sprite pipeline instance.
 */
Vec2F getSpritePipelineOffset(
	GraphicsPipeline spritePipeline);
/*
 * Sets sprite pipeline offset.
 *
 * spritePipeline - sprite pipeline instance.
 * offset - texture offset value.
 */
void setSpritePipelineOffset(
	GraphicsPipeline spritePipeline,
	Vec2F offset);

/*
 * Returns sprite pipeline color.
 * spritePipeline - sprite pipeline instance.
 */
LinearColor getSpritePipelineColor(
	GraphicsPipeline spritePipeline);
/*
 * Sets sprite pipeline color.
 *
 * spritePipeline - sprite pipeline instance.
 * color - color value.
 */
void setSpritePipelineColor(
	GraphicsPipeline spritePipeline,
	LinearColor color);
