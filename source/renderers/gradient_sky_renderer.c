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

#include "uran/renderers/gradient_sky_renderer.h"

struct GradientSkyAmbient_T
{
	LinearColor* colors;
	size_t count;
};

MpgxResult createGradientSkyAmbient(
	ImageData gradient,
	GradientSkyAmbient* gradientSkyAmbient)
{
	assert(gradient);
	assert(gradientSkyAmbient);

	Vec2I size = getImageDataSize(gradient);

	LinearColor* colors = malloc(
		sizeof(Vec4F) * size.x);

	if (!colors)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	const uint8_t* pixels = getImageDataPixels(gradient);

	for (int x = 0; x < size.x; x++)
	{
		LinearColor color = zeroLinearColor;

		for (int y = 0; y < size.y; y++)
		{
			size_t index = (y * size.x + x) * 4;

			LinearColor addition = srgbToLinearColor(srgbColor(
				pixels[index],
				pixels[index + 1],
				pixels[index + 2],
				pixels[index + 3]));
			color = addLinearColor(color, addition);
		}

		colors[x] = divValLinearColor(color, (float)size.y);
	}

	GradientSkyAmbient gradientSkyAmbientInstance = malloc(
		sizeof(GradientSkyAmbient_T));

	if (!gradientSkyAmbient)
	{
		free(colors);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	gradientSkyAmbientInstance->colors = colors;
	gradientSkyAmbientInstance->count = size.x;

	*gradientSkyAmbient = gradientSkyAmbientInstance;
	return SUCCESS_MPGX_RESULT;
}
void destroyGradientSkyAmbient(
	GradientSkyAmbient gradientSkyAmbient)
{
	if (!gradientSkyAmbient)
		return;

	free(gradientSkyAmbient->colors);
	free(gradientSkyAmbient);
}
LinearColor getGradientSkyAmbientColor(
	GradientSkyAmbient gradientSkyAmbient,
	float dayTime)
{
	assert(gradientSkyAmbient);
	assert(dayTime >= 0.0f);
	assert(dayTime <= 1.0f);

	LinearColor* colors = gradientSkyAmbient->colors;
	size_t colorCount = gradientSkyAmbient->count;

	dayTime = (float)(colorCount - 1) * dayTime;

	float secondValue = dayTime - (float)((int)dayTime);
	float firstValue = 1.0f - secondValue;

	LinearColor firstColor = colors[(size_t)dayTime];
	LinearColor secondColor = colors[(size_t)dayTime + 1];

	return linearColor(
		firstColor.r * firstValue + secondColor.r * secondValue,
		firstColor.g * firstValue + secondColor.g * secondValue,
		firstColor.b * firstValue + secondColor.b * secondValue,
		firstColor.a * firstValue + secondColor.a * secondValue);
}

