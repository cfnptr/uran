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
 * moveLeftKey - move left keyboard key.
 * moveRightKey - move right keyboard key.
 * moveDownKey - move down keyboard key.
 * moveUpKey - move up keyboard key.
 * moveBackwardKey - move backward keyboard key.
 * moveForwardKey - move forward keyboard key.
 */
FreeCamera createFreeCamera(
	Framebuffer framebuffer,
	Transformer transformer,
	cmmt_float_t moveSpeed,
	cmmt_float_t viewSpeed,
	cmmt_float_t fieldOfView,
	cmmt_float_t nearClipPlane,
	cmmt_float_t farClipPlane,
	int moveLeftKey,
	int moveRightKey,
	int moveDownKey,
	int moveUpKey,
	int moveBackwardKey,
	int moveForwardKey);
/*
 * Destroys free camera instance.
 * freeCamera - free camera instance or NULL.
 */
void destroyFreeCamera(FreeCamera freeCamera);

/*
 * Returns free camera framebuffer.
 * freeCamera - free camera instance.
 */
Framebuffer getFreeCameraFramebuffer(FreeCamera freeCamera);
/*
 * Returns free camera transform.
 * freeCamera - free camera instance.
 */
Transform getFreeCameraTransform(FreeCamera freeCamera);
/*
 * Returns free camera view direction.
 * freeCamera - free camera instance.
 */
Vec3F getFreeCameraViewDirection(FreeCamera freeCamera);

/*
 * Returns free camera position.
 * freeCamera - free camera instance.
 */
Vec3F getFreeCameraPosition(
	FreeCamera freeCamera);
/*
 * Sets free camera position.
 *
 * freeCamera - free camera instance.
 * position - camera position value.
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
 *
 * freeCamera - free camera instance.
 * rotation - camera rotation value.
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
 *
 * freeCamera - free camera instance.
 * moveSpeed - move speed value.
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
 *
 * freeCamera - free camera instance.
 * viewSpeed - view speed value.
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
 *
 * freeCamera - free camera instance.
 * fieldOfView - field of view value.
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
 *
 * freeCamera - free camera instance.
 * nearClipPlane - near clipping plane value.
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
 *
 * freeCamera - free camera instance.
 * farClipPlane - far clipping plane value.
 */
void setFreeCameraFarClipPlane(
	FreeCamera freeCamera,
	cmmt_float_t farClipPlane);

/*
 * Returns free camera move left keyboard key.
 * freeCamera - free camera instance.
 */
KeyboardKey getFreeCameraMoveLeftKey(
	FreeCamera freeCamera);
/*
 * Sets free camera move left keyboard key.
 *
 * freeCamera - free camera instance.
 * moveLeftKey - move left key value.
 */
void setFreeCameraMoveLeftKey(
	FreeCamera freeCamera,
	KeyboardKey moveLeftKey);

/*
 * Returns free camera move right keyboard key.
 * freeCamera - free camera instance.
 */
KeyboardKey getFreeCameraMoveRightKey(
	FreeCamera freeCamera);
/*
 * Sets free camera move right keyboard key.
 *
 * freeCamera - free camera instance.
 * moveRightKey - move right key value.
 */
void setFreeCameraMoveRightKey(
	FreeCamera freeCamera,
	KeyboardKey moveRightKey);

/*
 * Returns free camera move down keyboard key.
 * freeCamera - free camera instance.
 */
KeyboardKey getFreeCameraMoveDownKey(
	FreeCamera freeCamera);
/*
 * Sets free camera move down keyboard key.
 *
 * freeCamera - free camera instance.
 * moveDownKey - move down key value.
 */
void setFreeCameraMoveDownKey(
	FreeCamera freeCamera,
	KeyboardKey moveDownKey);

/*
 * Returns free camera move up keyboard key.
 * freeCamera - free camera instance.
 */
KeyboardKey getFreeCameraMoveUpKey(
	FreeCamera freeCamera);
/*
 * Sets free camera move up keyboard key.
 *
 * freeCamera - free camera instance.
 * moveUpKey - move up key value.
 */
void setFreeCameraMoveUpKey(
	FreeCamera freeCamera,
	KeyboardKey moveUpKey);

/*
 * Returns free camera move backward keyboard key.
 * freeCamera - free camera instance.
 */
KeyboardKey getFreeCameraMoveBackwardKey(
	FreeCamera freeCamera);
/*
 * Sets free camera move backward keyboard key.
 *
 * freeCamera - free camera instance.
 * moveBackwardKey - move backward key value.
 */
void setFreeCameraMoveBackwardKey(
	FreeCamera freeCamera,
	KeyboardKey moveBackwardKey);

/*
 * Returns free camera move forward keyboard key.
 * freeCamera - free camera instance.
 */
KeyboardKey getFreeCameraMoveForwardKey(
	FreeCamera freeCamera);
/*
 * Sets free camera move forward keyboard key.
 *
 * freeCamera - free camera instance.
 * moveForwardKey - move forward key value.
 */
void setFreeCameraMoveForwardKey(
	FreeCamera freeCamera,
	KeyboardKey moveForwardKey);

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
