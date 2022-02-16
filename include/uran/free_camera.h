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
#include "uran/transformer.h"

#include "mpgx/window.h"
#include "cmmt/camera.h"

/*
 * Free camera structure.
 */
typedef struct FreeCamera_T FreeCamera_T;
/*
 * Free camera instance.
 */
typedef FreeCamera_T* FreeCamera;


FreeCamera createFreeCamera(
	Framebuffer framebuffer,
	Transformer transformer,
	cmmt_float_t moveSpeed,
	cmmt_float_t viewSpeed,
	cmmt_float_t fieldOfView,
	cmmt_float_t nearClipPlane,
	cmmt_float_t farClipPlane);
FreeCamera createDefaultFreeCamera(
	Framebuffer framebuffer,
	Transformer transformer);
void destroyFreeCamera(FreeCamera freeCamera);

Framebuffer getFreeCameraFramebuffer(
	FreeCamera freeCamera);
Vec3F getFreeCameraViewDirection(
	FreeCamera freeCamera);

Vec3F getFreeCameraPosition(
	FreeCamera freeCamera);
void setFreeCameraPosition(
	FreeCamera freeCamera,
	Vec3F position);

Vec2F getFreeCameraRotation(
	FreeCamera freeCamera);
void setFreeCameraRotation(
	FreeCamera freeCamera,
	Vec2F rotation);

cmmt_float_t getFreeCameraMoveSpeed(
	FreeCamera freeCamera);
void setFreeCameraMoveSpeed(
	FreeCamera freeCamera,
	cmmt_float_t moveSpeed);

cmmt_float_t getFreeCameraViewSpeed(
	FreeCamera freeCamera);
void setFreeCameraViewSpeed(
	FreeCamera freeCamera,
	cmmt_float_t viewSpeed);

cmmt_float_t getFreeCameraFieldOfView(
	FreeCamera freeCamera);
void setFreeCameraFieldOfView(
	FreeCamera freeCamera,
	cmmt_float_t fieldOfView);

cmmt_float_t getFreeCameraNearClipPlane(
	FreeCamera freeCamera);
void setFreeCameraNearClipPlane(
	FreeCamera freeCamera,
	cmmt_float_t nearClipPlane);

cmmt_float_t getFreeCameraFarClipPlane(
	FreeCamera freeCamera);
void setFreeCameraFarClipPlane(
	FreeCamera freeCamera,
	cmmt_float_t farClipPlane);

void updateFreeCamera(FreeCamera freeCamera);
Camera getFreeCamera(FreeCamera freeCamera);
