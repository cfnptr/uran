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
#include <stdint.h>

/*
 * 2D square vertices. (float)
 */
static const float squareVertices2D[] = {
	-1.0f, -1.0f,
	-1.0f, 1.0f,
	1.0f, 1.0f,
	1.0f, -1.0f,
};
/*
 * 2D square vertices, texture coordinates. (float)
 */
static const float squareVerticesCoords2D[] = {
	-1.0f, -1.0f, 0.0f, 0.0f,
	-1.0f, 1.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, -1.0f, 1.0f, 0.0f,
};
/*
 * 3D square vertices. (float)
 */
static const float squareVertices3D[] = {
	-1.0f, -1.0f, 0.0f,
	-1.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
};
/*
 * 3D square vertices, normals. (float)
 */
static const float squareVerticesNormals3D[] = {
	-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
	-1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
	1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
	1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
};
/*
 * 3D square vertices, texture coordinates. (float)
 */
static const float squareVerticesCoords3D[] = {
	-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
	-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
	1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
};
/*
 * 3D square vertices, normals, texture coordinates. (float)
 */
static const float squareVerticesNormalsCoords3D[] = {
	-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
	-1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
	1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
};
/*
 * One size 2D square vertices. (float)
 */
static const float oneSquareVertices2D[] = {
	-0.5f, -0.5f,
	-0.5f, 0.5f,
	0.5f, 0.5f,
	0.5f, -0.5f,
};
/*
 * One size 2D square vertices, texture coordinates. (float)
 */
static const float oneSquareVerticesCoords2D[] = {
	-0.5f, -0.5f, 0.0f, 0.0f,
	-0.5f, 0.5f, 0.0f, 1.0f,
	0.5f, 0.5f, 1.0f, 1.0f,
	0.5f, -0.5f, 1.0f, 0.0f,
};
/*
 * Triangle square indices. (uint16)
 */
static const uint16_t triangleSquareIndices[] = {
	0, 1, 2, 0, 2, 3,
};
/*
 * Line square indices. (uint16)
 */
static const uint16_t lineSquareIndices[] = {
	0, 1, 1, 2, 2, 3, 3, 0,
};
