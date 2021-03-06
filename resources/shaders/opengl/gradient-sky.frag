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

in vec3 f_FragDir;
in float f_TexCoords;

layout(location = 0) out vec4 o_Color;

uniform vec4 u_SunDirSize;
uniform vec4 u_SunColor;
uniform sampler2D u_Texture;

vec4 calcSkyColor(sampler2D skyTexture, float sunHeight, float texCoord)
{
    vec2 texCoords = vec2(max(sunHeight, 0.0), texCoord);
    return texture(skyTexture, texCoords);
}
float calcSunLight(vec3 fragDir, vec3 sunDir, float size)
{
    float light = dot(normalize(fragDir), sunDir);
    return pow(max(light, 0.0), size);
}
void main()
{
    vec4 skyColor = calcSkyColor(u_Texture, u_SunDirSize.y, f_TexCoords);
    float sunLight = calcSunLight(f_FragDir, u_SunDirSize.xyz, u_SunDirSize.w);
    o_Color = (u_SunColor * sunLight) + skyColor;
    gl_FragDepth = 0.9999999;
}
