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

layout(location = 0) in vec3 f_Normal;
layout(location = 0) out vec4 o_Color;

layout(binding = 0) uniform UniformBuffer
{
    vec4 objectColor;
    vec4 ambientColor;
    vec4 lightColor;
    vec4 lightDirection;
} ub;

float calcDiffuse(vec3 normal, vec3 lightDirection)
{
    return max(dot(normal, -lightDirection), 0.0);
}
void main()
{
    vec4 ambientColor = ub.objectColor * ub.ambientColor;
    float diffuse = calcDiffuse(f_Normal, ub.lightDirection.xyz);
    vec4 diffuseColor = ub.lightColor * diffuse;
    o_Color = (ambientColor + diffuseColor) * ub.objectColor;
}
