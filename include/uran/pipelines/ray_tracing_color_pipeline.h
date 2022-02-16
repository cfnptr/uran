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

#define RAY_TRACING_COLOR_PIPELINE_NAME "RayTracingColor"

MpgxResult createRayTracingColorPipeline(
	Window window,
	Shader generationShader,
	Shader missShader,
	Shader closestHitShader,
	RayTracingScene scene,
	RayTracingPipeline* rayTracingColorPipeline);

RayTracingScene getRayTracingColorPipelineScene(
	RayTracingPipeline rayTracingColorPipeline);

Mat4F getRayTracingColorPipelineInvView(
	RayTracingPipeline rayTracingColorPipeline);
void setRayTracingColorPipelineInvView(
	RayTracingPipeline rayTracingColorPipeline,
	Mat4F invView);

Mat4F getRayTracingColorPipelineInvProj(
	RayTracingPipeline rayTracingColorPipeline);
void setRayTracingColorPipelineInvProj(
	RayTracingPipeline rayTracingColorPipeline,
	Mat4F invProj);
