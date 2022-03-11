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

#define TEXTURE_COLOR_PIPELINE_NAME "TextureColor"

/*
 * Create a new texture color pipeline instance.
 * Returns operation MPGX result.
 *
 * framebuffer - framebuffer instance.
 * vertexShader - texture color vertex shader.
 * fragmentShader - texture color fragment shader.
 * texture - texture instance.
 * sampler - texture sampler.
 * state - pipeline state or NULL.
 * textureColorPipeline - pointer to the texture color pipeline.
 */
MpgxResult createTextureColorPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	const GraphicsPipelineState* state,
	GraphicsPipeline* textureColorPipeline);

/*
 * Returns texture color pipeline texture.
 * textureColorPipeline - texture color pipeline instance.
 */
Image getTextureColorPipelineTexture(
	GraphicsPipeline textureColorPipeline);
/*
 * Returns texture color pipeline sampler.
 * textureColorPipeline - texture color pipeline instance.
 */
Sampler getTextureColorPipelineSampler(
	GraphicsPipeline textureColorPipeline);

/*
 * Returns texture color pipeline model view projection matrix.
 * textureColorPipeline - texture color pipeline instance.
 */
Mat4F getTextureColorPipelineMvp(
	GraphicsPipeline textureColorPipeline);
/*
 * Returns texture color pipeline model view projection matrix.
 *
 * textureColorPipeline - texture color pipeline instance.
 * mvp - model view projection matrix value.
 */
void setTextureColorPipelineMvp(
	GraphicsPipeline textureColorPipeline,
	Mat4F mvp);

/*
 * Returns texture color pipeline size.
 * textureColorPipeline - texture color pipeline instance.
 */
Vec2F getTextureColorPipelineSize(
	GraphicsPipeline textureColorPipeline);
/*
 * Sets texture color pipeline size.
 *
 * textureColorPipeline - texture color pipeline instance.
 * size - texture size value.
 */
void setTextureColorPipelineSize(
	GraphicsPipeline textureColorPipeline,
	Vec2F size);

/*
 * Returns texture color pipeline offset.
 * textureColorPipeline - texture color pipeline instance.
 */
Vec2F getTextureColorPipelineOffset(
	GraphicsPipeline textureColorPipeline);
/*
 * Sets texture color pipeline offset.
 *
 * textureColorPipeline - texture color pipeline instance.
 * offset - texture offset value.
 */
void setTextureColorPipelineOffset(
	GraphicsPipeline textureColorPipeline,
	Vec2F offset);

/*
 * Returns texture color pipeline color.
 * textureColorPipeline - texture color pipeline instance.
 */
LinearColor getTextureColorPipelineColor(
	GraphicsPipeline textureColorPipeline);
/*
 * Sets texture color pipeline color.
 *
 * textureColorPipeline - texture color pipeline instance.
 * color - color value.
 */
void setTextureColorPipelineColor(
	GraphicsPipeline textureColorPipeline,
	LinearColor color);
