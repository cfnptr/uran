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

#define RAY_COLOR_PIPELINE_NAME "RayColor"

// TODO: fix and add comments

MpgxResult createRayColorPipeline(
	Window window,
	Shader generationShader,
	Shader missShader,
	Shader closestHitShader,
	RayTracingScene scene,
	RayTracingPipeline* rayColorPipeline);

RayTracingScene getRayColorPipelineScene(
	RayTracingPipeline rayColorPipeline);

const mat4* getRayColorPipelineInvView(
	RayTracingPipeline rayColorPipeline);
void setRayColorPipelineInvView(
	RayTracingPipeline rayColorPipeline,
	const Mat4F* invView);

const mat4* getRayColorPipelineInvProj(
	RayTracingPipeline rayColorPipeline);
void setRayColorPipelineInvProj(
	RayTracingPipeline rayColorPipeline,
	const Mat4F* invProj);
