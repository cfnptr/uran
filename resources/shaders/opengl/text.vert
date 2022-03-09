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

layout(location = 0) in vec2 v_Position;
layout(location = 1) in vec3 v_TexCoords;
layout(location = 2) in uvec4 v_Color;

out vec2 f_TexCoords;
flat out int f_AtlasIndex;
flat out vec4 f_Color;

uniform mat4 u_MVP;

vec4 srgbToLinear(vec4 srgb)
{
    bvec3 cutoff = lessThanEqual(srgb.rgb, vec3(0.04045));
    vec3 higher = pow((srgb.rgb + vec3(0.055)) / vec3(1.055), vec3(2.4));
    vec3 lower = srgb.rgb / vec3(12.92);
    return vec4(mix(higher, lower, cutoff), srgb.a);
}

void main()
{
    gl_Position = u_MVP * vec4(v_Position, 0.0, 1.0);
    f_TexCoords = v_TexCoords.xy;
    f_AtlasIndex = int(v_TexCoords.z);
    f_Color = srgbToLinear(vec4(v_Color) * (1.0 / 255.0));
}
