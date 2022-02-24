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

#version 420

layout(location = 0) in vec2 f_TexCoords;
layout(location = 1) flat in int f_Atlas;
layout(location = 2) flat in vec4 f_Color;

layout(location = 0) out vec4 o_Color;
layout(binding = 0) uniform sampler2D u_Atlases[4];

void main()
{
    float alpha = texture(u_Atlases[f_Atlas], f_TexCoords).r;
    o_Color = vec4(f_Color.rgb, f_Color.a * alpha);
}
