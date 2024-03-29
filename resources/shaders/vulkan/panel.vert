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

#version 410

layout(location = 0) in vec2 v_Position;

layout(push_constant) uniform VertexPushConstants
{
	mat4 mvp;
} vpc;

void main()
{
	gl_Position = vpc.mvp * vec4(v_Position, 0.0, 1.0);
}
