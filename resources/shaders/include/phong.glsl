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

#ifndef PHONG_H
#define PHONG_H

float calcDiffuse(vec3 normal, vec3 lightDirection)
{
    return max(dot(normal, -lightDirection), 0.0);
}

vec3 calcViewDir(vec3 cameraPos, vec3 fargPos)
{
    return normalize(cameraPos - fargPos);
}
float calcSpecular(
    float specular, vec3 viewDir,
    vec3 lightDir, vec3 normal)
{
    if (specular == 0.0)
        return 0.0;
    
    vec3 reflDir = reflect(lightDir, normal);
    return pow(max(dot(viewDir, reflDir), 0.0), specular);
}

#endif
