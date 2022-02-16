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

#define SIMPLE_SHADOW_PIPELINE_NAME "SimpleShadow"

MpgxResult createSimpleShadowSampler(
	Window window,
	Sampler* simpleShadowSampler);

MpgxResult createSimpleShadowPipelineExt(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	const GraphicsPipelineState* state,
	GraphicsPipeline* simpleShadowPipeline);
MpgxResult createSimpleShadowPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	int32_t shadowMapLength,
	GraphicsPipeline* simpleShadowPipeline);

Mat4F getSimpleShadowPipelineMvp(
	GraphicsPipeline simpleShadowPipeline);
void setSimpleShadowPipelineMvp(
	GraphicsPipeline simpleShadowPipeline,
	Mat4F mvp);
