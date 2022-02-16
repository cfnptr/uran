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

#define COLOR_PIPELINE_NAME "Color"

MpgxResult createColorPipelineExt(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	const GraphicsPipelineState* state,
	GraphicsPipeline* colorPipeline);
MpgxResult createColorPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	GraphicsPipeline* colorPipeline);

Mat4F getColorPipelineMvp(
	GraphicsPipeline colorPipeline);
void setColorPipelineMvp(
	GraphicsPipeline colorPipeline,
	Mat4F mvp);

LinearColor getColorPipelineColor(
	GraphicsPipeline colorPipeline);
void setColorPipelineColor(
	GraphicsPipeline colorPipeline,
	LinearColor color);
