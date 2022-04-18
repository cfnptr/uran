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
#include "pack/reader.h"
#include "logy/logger.h"
#include "cmmt/color.h"

#define PANEL_PIPELINE_NAME "Panel"

/*
 * Create a new panel pipeline instance.
 * Returns operation MPGX result.
 *
 * framebuffer - framebuffer instance.
 * vertexShader - panel vertex shader.
 * fragmentShader - panel fragment shader.
 * mesh - panel mesh.
 * state - pipeline state or NULL.
 * useScissors - use scissors for text rendering.
 * panelPipeline - pointer to the panel pipeline.
 */
MpgxResult createPanelPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	GraphicsMesh mesh,
	const GraphicsPipelineState* state,
	bool useScissors,
	GraphicsPipeline* panelPipeline);

/*
 * Returns panel pipeline mesh.
 * panelPipeline - panel pipeline instance.
 */
GraphicsMesh getPanelPipelineMesh(
	GraphicsPipeline panelPipeline);
/*
 * Sets panel pipeline mesh.
 *
 * panelPipeline - panel pipeline instance.
 * mesh - panel mesh.
 */
void setPanelPipelineMesh(
	GraphicsPipeline panelPipeline,
	GraphicsMesh mesh);

/*
 * Returns panel pipeline model view projection matrix.
 * panelPipeline - panel pipeline instance.
 */
Mat4F getPanelPipelineMvp(
	GraphicsPipeline panelPipeline);
/*
 * Sets panel pipeline model view projection matrix.
 *
 * panelPipeline - panel pipeline instance.
 * mvp - model view projection matrix value.
 */
void setPanelPipelineMvp(
	GraphicsPipeline panelPipeline,
	Mat4F mvp);

/*
 * Returns panel pipeline color.
 * panelPipeline - panel pipeline instance.
 */
LinearColor getPanelPipelineColor(
	GraphicsPipeline panelPipeline);
/*
 * Sets panel pipeline color.
 *
 * panelPipeline - panel pipeline instance.
 * color - color value.
 */
void setPanelPipelineColor(
	GraphicsPipeline panelPipeline,
	LinearColor color);
