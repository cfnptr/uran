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

MpgxResult createTextureSpritePipelineExt(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	const GraphicsPipelineState* state,
	GraphicsPipeline* textureSpritePipeline);
MpgxResult createTextureSpritePipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	GraphicsPipeline* textureSpritePipeline);

Image getTextureSpritePipelineTexture(
	GraphicsPipeline textureSpritePipeline);
Sampler getTextureSpritePipelineSampler(
	GraphicsPipeline textureSpritePipeline);

Mat4F getTextureSpritePipelineMvp(
	GraphicsPipeline textureSpritePipeline);
void setTextureSpritePipelineMvp(
	GraphicsPipeline textureSpritePipeline,
	Mat4F mvp);

Vec2F getTextureSpritePipelineSize(
	GraphicsPipeline textureSpritePipeline);
void setTextureSpritePipelineSize(
	GraphicsPipeline textureSpritePipeline,
	Vec2F size);

Vec2F getTextureSpritePipelineOffset(
	GraphicsPipeline textureSpritePipeline);
void setTextureSpritePipelineOffset(
	GraphicsPipeline textureSpritePipeline,
	Vec2F offset);

LinearColor getTextureSpritePipelineColor(
	GraphicsPipeline textureSpritePipeline);
void setTextureSpritePipelineColor(
	GraphicsPipeline textureSpritePipeline,
	LinearColor color);
