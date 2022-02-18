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

/*
 * Create a new free camera instance.
 * Returns free camera instance on success, otherwise NULL.
 *
 * framebuffer - framebuffer instance.
 * transformer - transformer instance.
 * moveSpeed - move speed multiplier.
 * viewSpeed - view speed multiplier.
 * fieldOfView - field of view in radians.
 * nearClipPlane - near clipping plane.
 * farClipPlane - far clipping plane.
 */
FreeCamera createFreeCamera(
	Framebuffer framebuffer,
	Transformer transformer,
	cmmt_float_t moveSpeed,
	cmmt_float_t viewSpeed,
	cmmt_float_t fieldOfView,
	cmmt_float_t nearClipPlane,
	cmmt_float_t farClipPlane);
/*
 * Create a new default free camera instance.
 * Returns free camera instance on success, otherwise NULL.
 *
 * framebuffer - framebuffer instance.
 * transformer - transformer instance.
 */
FreeCamera createDefaultFreeCamera(
	Framebuffer framebuffer,
	Transformer transformer);
/*
 * Destroys free camera instance.
 * freeCamera - free camera instance or NULL.
 */
void destroyFreeCamera(FreeCamera freeCamera);

/*
 * Returns free camera view direction.
 * freeCamera - free camera instance.
 */
Vec3F getFreeCameraViewDirection(
	FreeCamera freeCamera);

/*
 * Returns free camera framebuffer.
 * freeCamera - free camera instance.
 */
Framebuffer getFreeCameraFramebuffer(
	FreeCamera freeCamera);

/*
 * Returns free camera position.
 * freeCamera - free camera instance.
 */
Vec3F getFreeCameraPosition(
	FreeCamera freeCamera);
/*
 * Sets free camera position.
 * freeCamera - free camera instance.
 */
void setFreeCameraPosition(
	FreeCamera freeCamera,
	Vec3F position);

/*
 * Returns free camera rotation.
 * freeCamera - free camera instance.
 */
Vec2F getFreeCameraRotation(
	FreeCamera freeCamera);
/*
 * Sets free camera rotation.
 * freeCamera - free camera instance.
 */
void setFreeCameraRotation(
	FreeCamera freeCamera,
	Vec2F rotation);

/*
 * Returns free camera move speed multiplier.
 * freeCamera - free camera instance.
 */
cmmt_float_t getFreeCameraMoveSpeed(
	FreeCamera freeCamera);
/*
 * Sets free camera move speed multiplier.
 * freeCamera - free camera instance.
 */
void setFreeCameraMoveSpeed(
	FreeCamera freeCamera,
	cmmt_float_t moveSpeed);

/*
 * Returns free camera view speed multiplier.
 * freeCamera - free camera instance.
 */
cmmt_float_t getFreeCameraViewSpeed(
	FreeCamera freeCamera);
/*
 * Sets free camera view speed multiplier.
 * freeCamera - free camera instance.
 */
void setFreeCameraViewSpeed(
	FreeCamera freeCamera,
	cmmt_float_t viewSpeed);

/*
 * Returns free camera field of view in radians.
 * freeCamera - free camera instance.
 */
cmmt_float_t getFreeCameraFieldOfView(
	FreeCamera freeCamera);
/*
 * Sets free camera field of view in radians.
 * freeCamera - free camera instance.
 */
void setFreeCameraFieldOfView(
	FreeCamera freeCamera,
	cmmt_float_t fieldOfView);

/*
 * Returns free camera near clipping plane.
 * freeCamera - free camera instance.
 */
cmmt_float_t getFreeCameraNearClipPlane(
	FreeCamera freeCamera);
/*
 * Sets free camera near clipping plane.
 * freeCamera - free camera instance.
 */
void setFreeCameraNearClipPlane(
	FreeCamera freeCamera,
	cmmt_float_t nearClipPlane);

/*
 * Returns free camera far clipping plane.
 * freeCamera - free camera instance.
 */
cmmt_float_t getFreeCameraFarClipPlane(
	FreeCamera freeCamera);
/*
 * Sets free camera far clipping plane.
 * freeCamera - free camera instance.
 */
void setFreeCameraFarClipPlane(
	FreeCamera freeCamera,
	cmmt_float_t farClipPlane);

/*
 * Updates camera position and rotation.
 * freeCamera - free camera instance.
 */
void updateFreeCamera(FreeCamera freeCamera);
/*
 * Returns free camera value.
 * freeCamera - free camera instance.
 */
Camera getFreeCamera(FreeCamera freeCamera);
