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

MpgxResult createDiffusePipelineExt(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	const GraphicsPipelineState* state,
	GraphicsPipeline* diffusePipeline);
MpgxResult createDiffusePipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	GraphicsPipeline* diffusePipeline);

Mat4F getDiffusePipelineMvp(
	GraphicsPipeline diffusePipeline);
void setDiffusePipelineMvp(
	GraphicsPipeline diffusePipeline,
	Mat4F mvp);

Mat4F getDiffusePipelineNormal(
	GraphicsPipeline diffusePipeline);
void setDiffusePipelineNormal(
	GraphicsPipeline diffusePipeline,
	Mat4F normal);

LinearColor getDiffusePipelineObjectColor(
	GraphicsPipeline diffusePipeline);
void setDiffusePipelineObjectColor(
	GraphicsPipeline diffusePipeline,
	LinearColor objectColor);

LinearColor getDiffusePipelineAmbientColor(
	GraphicsPipeline diffusePipeline);
void setDiffusePipelineAmbientColor(
	GraphicsPipeline diffusePipeline,
	LinearColor ambientColor);

LinearColor getDiffusePipelineLightColor(
	GraphicsPipeline diffusePipeline);
void setDiffusePipelineLightColor(
	GraphicsPipeline diffusePipeline,
	LinearColor lightColor);

Vec3F getDiffusePipelineLightDirection(
	GraphicsPipeline diffusePipeline);
void setDiffusePipelineLightDirection(
	GraphicsPipeline diffusePipeline,
	Vec3F lightDirection);
