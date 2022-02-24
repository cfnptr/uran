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

in vec2 f_TexCoords;
flat in int f_Atlas;
flat in vec4 f_Color;

layout(location = 0) out vec4 o_Color;

uniform sampler2D u_RegularAtlas;
uniform sampler2D u_BoldAtlas;
uniform sampler2D u_ItalicAtlas;
uniform sampler2D u_BoldItalicAtlas;

void main()
{
    float alpha;
    
    if (f_Atlas == 0)
        alpha = texture(u_RegularAtlas, f_TexCoords).r;
    else if (f_Atlas == 1)
        alpha = texture(u_BoldAtlas, f_TexCoords).r;
    else if (f_Atlas == 2)
        alpha = texture(u_ItalicAtlas, f_TexCoords).r;
    else
        alpha = texture(u_BoldItalicAtlas, f_TexCoords).r;
    
    o_Color = vec4(f_Color.rgb, f_Color.a * alpha);
}
