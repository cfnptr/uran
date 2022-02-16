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

#ifndef SHADOW_H
#define SHADOW_H

float calcShadow(vec3 shadowCoords, sampler2DShadow shadowMap)
{
    if(shadowCoords.z > 1.0)
        return 1.0;
    
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    
    float shadow = 0.0;
    
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            vec2 coords = shadowCoords.xy + vec2(x, y) * texelSize;
            shadow += texture(shadowMap, vec3(coords, shadowCoords.z));
        }
    }
    
    return shadow * (1.0 / 9.0);
}

#endif
