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

#define BLOOM_PIPELINE_NAME "Bloom"

MpgxResult createBloomPipelineExt(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image buffer,
	Sampler sampler,
	const GraphicsPipelineState* state,
	GraphicsPipeline* bloomPipeline);
MpgxResult createBloomPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image buffer,
	Sampler sampler,
	GraphicsPipeline* bloomPipeline);

Image getBloomPipelineBuffer(
	GraphicsPipeline bloomPipeline);
Sampler getBloomPipelineSampler(
	GraphicsPipeline bloomPipeline);

LinearColor getBloomPipelineThreshold(
	GraphicsPipeline bloomPipeline);
void setBloomPipelineThreshold(
	GraphicsPipeline bloomPipeline,
	LinearColor threshold);
