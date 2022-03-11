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

#define TEXTURE_SPRITE_PIPELINE_NAME "TextureSprite"

/*
 * Create a new texture sprite pipeline instance.
 * Returns operation MPGX result.
 *
 * framebuffer - framebuffer instance.
 * vertexShader - texture sprite vertex shader.
 * fragmentShader - texture sprite fragment shader.
 * texture - texture instance.
 * sampler - texture sampler.
 * state - pipeline state or NULL.
 * textureSpritePipeline - pointer to the texture sprite pipeline.
 */
MpgxResult createTextureSpritePipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	const GraphicsPipelineState* state,
	GraphicsPipeline* textureSpritePipeline);

/*
 * Returns texture sprite pipeline texture.
 * textureSpritePipeline - texture sprite pipeline instance.
 */
Image getTextureSpritePipelineTexture(
	GraphicsPipeline textureSpritePipeline);
/*
 * Returns texture sprite pipeline sampler.
 * textureSpritePipeline - texture sprite pipeline instance.
 */
Sampler getTextureSpritePipelineSampler(
	GraphicsPipeline textureSpritePipeline);

/*
 * Returns texture sprite pipeline model view projection matrix.
 * textureSpritePipeline - texture sprite pipeline instance.
 */
Mat4F getTextureSpritePipelineMvp(
	GraphicsPipeline textureSpritePipeline);
/*
 * Sets texture sprite pipeline model view projection matrix.
 *
 * textureSpritePipeline - texture sprite pipeline instance.
 * mvp - model view projection matrix.
 */
void setTextureSpritePipelineMvp(
	GraphicsPipeline textureSpritePipeline,
	Mat4F mvp);

/*
 * Returns texture sprite pipeline size.
 * textureSpritePipeline - texture sprite pipeline instance.
 */
Vec2F getTextureSpritePipelineSize(
	GraphicsPipeline textureSpritePipeline);
/*
 * Sets texture sprite pipeline size.
 *
 * textureSpritePipeline - texture sprite pipeline instance.
 * size - texture size value.
 */
void setTextureSpritePipelineSize(
	GraphicsPipeline textureSpritePipeline,
	Vec2F size);

/*
 * Returns texture sprite pipeline offset.
 * textureSpritePipeline - texture sprite pipeline instance.
 */
Vec2F getTextureSpritePipelineOffset(
	GraphicsPipeline textureSpritePipeline);
/*
 * Sets texture sprite pipeline offset.
 *
 * textureSpritePipeline - texture sprite pipeline instance.
 * offset - texture offset value.
 */
void setTextureSpritePipelineOffset(
	GraphicsPipeline textureSpritePipeline,
	Vec2F offset);

/*
 * Returns texture sprite pipeline color.
 * textureSpritePipeline - texture sprite pipeline instance.
 */
LinearColor getTextureSpritePipelineColor(
	GraphicsPipeline textureSpritePipeline);
/*
 * Sets texture sprite pipeline color.
 *
 * textureSpritePipeline - texture sprite pipeline instance.
 * color - color value.
 */
void setTextureSpritePipelineColor(
	GraphicsPipeline textureSpritePipeline,
	LinearColor color);
