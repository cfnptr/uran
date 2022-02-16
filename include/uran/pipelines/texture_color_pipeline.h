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

MpgxResult createTextureColorPipelineExt(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	const GraphicsPipelineState* state,
	GraphicsPipeline* textureColorPipeline);
MpgxResult createTextureColorPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	GraphicsPipeline* textureColorPipeline);

Image getTextureColorPipelineTexture(
	GraphicsPipeline textureColorPipeline);
Sampler getTextureColorPipelineSampler(
	GraphicsPipeline textureColorPipeline);

Mat4F getTextureColorPipelineMvp(
	GraphicsPipeline textureColorPipeline);
void setTextureColorPipelineMvp(
	GraphicsPipeline textureColorPipeline,
	Mat4F mvp);

Vec2F getTextureColorPipelineSize(
	GraphicsPipeline textureColorPipeline);
void setTextureColorPipelineSize(
	GraphicsPipeline textureColorPipeline,
	Vec2F size);

Vec2F getTextureColorPipelineOffset(
	GraphicsPipeline textureColorPipeline);
void setTextureColorPipelineOffset(
	GraphicsPipeline textureColorPipeline,
	Vec2F offset);

LinearColor getTextureColorPipelineColor(
	GraphicsPipeline textureColorPipeline);
void setTextureColorPipelineColor(
	GraphicsPipeline textureColorPipeline,
	LinearColor color);
