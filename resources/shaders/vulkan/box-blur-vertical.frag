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
layout(location = 0) out vec4 o_Color;

layout(push_constant) uniform FragmentPushConstants
{
    int radius;
} fpc;

layout(binding = 0) uniform sampler2D u_Buffer;

vec3 calcBlurColor(sampler2D buffer, vec2 texCoords, int radius)
{
    vec2 texelSize = 1.0 / textureSize(buffer, 0);
    vec3 result = vec3(0.0);
    
    for (int i = -radius; i <= radius; i++)
    {
        vec2 coords = vec2(0.0, texelSize.y * i);
        result += texture(buffer, texCoords + coords).rgb;
    }
    
    return result / (radius + radius + 1);
}
void main()
{
    vec3 blurColor = calcBlurColor(
        u_Buffer, f_TexCoords, fpc.radius);
    o_Color = vec4(blurColor, 1.0);
}
