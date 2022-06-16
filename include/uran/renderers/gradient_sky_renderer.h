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

#pragma once
#include "uran/image_data.h"

typedef struct GradientSkyAmbient_T GradientSkyAmbient_T;
typedef GradientSkyAmbient_T* GradientSkyAmbient;

/*
 * Creates a new gradient sky ambient instance.
 * gradient - gradient image data.
 */
GradientSkyAmbient createGradientSkyAmbient(ImageData gradient);
/*
 * Destroys gradient sky ambient instance.
 * gradientSkyAmbient - gradient sky ambient instance or NULL.
 */
void destroyGradientSkyAmbient(GradientSkyAmbient gradientSkyAmbient);

/*
 * Returns gradient sky ambient color sample.
 *
 * gradientSkyAmbient - gradient sky ambient instance.
 * dayTime - day time value. (in 0.0 - 1.0 range)
 */
LinearColor getGradientSkyAmbientColor(
	GradientSkyAmbient gradientSkyAmbient,
	cmmt_float_t dayTime);
