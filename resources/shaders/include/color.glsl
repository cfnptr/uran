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

#ifndef COLOR_H
#define COLOR_H

vec3 srgbToLinear(vec3 srgb)
{
    bvec3 cutoff = lessThanEqual(srgb, vec3(0.04045));
    vec3 higher = pow((srgb + vec3(0.055)) / vec3(1.055), vec3(2.4));
    vec3 lower = srgb / vec3(12.92);
    return mix(higher, lower, cutoff);
}
vec4 srgbToLinear(vec4 srgb)
{
    bvec3 cutoff = lessThanEqual(srgb.rgb, vec3(0.04045));
    vec3 higher = pow((srgb.rgb + vec3(0.055)) / vec3(1.055), vec3(2.4));
    vec3 lower = srgb.rgb / vec3(12.92);
    return vec4(mix(higher, lower, cutoff), srgb.a);
}

vec3 linearToSrgb(vec3 linear)
{
    bvec3 cutoff = lessThanEqual(linear, vec3(0.0031308));
    vec3 higher = vec3(1.055) * pow(linear, vec3(1.0 / 2.4)) - vec3(0.055);
    vec3 lower = linear * vec3(12.92);
    return mix(higher, lower, cutoff);
}
vec4 linearToSrgb(vec4 linear)
{
    bvec3 cutoff = lessThanEqual(linear.rgb, vec3(0.0031308));
    vec3 higher = vec3(1.055) * pow(linear.rgb, vec3(1.0 / 2.4)) - vec3(0.055);
    vec3 lower = linear.rgb * vec3(12.92);
    return vec4(mix(higher, lower, cutoff), linear.a);
}

#endif
