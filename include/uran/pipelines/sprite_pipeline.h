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

MpgxResult createSpritePipelineExt(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	const GraphicsPipelineState* state,
	GraphicsPipeline* spritePipeline);
MpgxResult createSpritePipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	GraphicsPipeline* spritePipeline);

Mat4F getSpritePipelineMvp(
	GraphicsPipeline spritePipeline);
void setSpritePipelineMvp(
	GraphicsPipeline spritePipeline,
	Mat4F mvp);

LinearColor getSpritePipelineColor(
	GraphicsPipeline spritePipeline);
void setSpritePipelineColor(
	GraphicsPipeline spritePipeline,
	LinearColor color);
