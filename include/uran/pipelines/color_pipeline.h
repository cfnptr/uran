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

/*
 * Create a new color pipeline instance.
 * Returns operation MPGX result.
 *
 * framebuffer - framebuffer instance.
 * vertexShader - color vertex shader.
 * fragmentShader - color fragment shader.
 * state - pipeline state or NULL.
 * colorPipeline - pointer to the color pipeline.
 */
MpgxResult createColorPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	const GraphicsPipelineState* state,
	GraphicsPipeline* colorPipeline);

/*
 * Returns color pipeline model view projection matrix.
 * colorPipeline - color pipeline instance.
 */
const mat4* getColorPipelineMvp(
	GraphicsPipeline colorPipeline);
/*
 * Sets color pipeline model view projection matrix.
 *
 * colorPipeline - color pipeline instance.
 * mvp - model view projection matrix value.
 */
void setColorPipelineMvp(
	GraphicsPipeline colorPipeline,
	const Mat4F* mvp);

/*
 * Returns color pipeline color.
 * colorPipeline - color pipeline instance.
 */
vec4 getColorPipelineColor(
	GraphicsPipeline colorPipeline);
/*
 * Sets color pipeline color.
 *
 * colorPipeline - color pipeline instance.
 * color - color value.
 */
void setColorPipelineColor(
	GraphicsPipeline colorPipeline,
	LinearColor color);
