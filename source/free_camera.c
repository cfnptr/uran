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

#include "uran/free_camera.h"

#include "cmmt/angle.h"
#include "cmmt/common.h"

#include <stdlib.h>
#include <assert.h>

#define LERP_FACTOR (cmmt_float_t)20.0 // TODO: set

// TODO: boost (shift key) and boost speed

struct FreeCamera_T
{
	Framebuffer framebuffer;
	Transform transform;
	Vec2F rotation;
	Vec2F lastCursorPosition;
	Vec3F velocity;
	cmmt_float_t moveSpeed;
	cmmt_float_t viewSpeed;
	cmmt_float_t fieldOfView;
	cmmt_float_t nearClipPlane;
	cmmt_float_t farClipPlane;
	KeyboardKey moveLeftKey;
	KeyboardKey moveRightKey;
	KeyboardKey moveDownKey;
	KeyboardKey moveUpKey;
	KeyboardKey moveBackwardKey;
	KeyboardKey moveForwardKey;
};

FreeCamera createFreeCamera(
	Framebuffer framebuffer,
	Transformer transformer,
	cmmt_float_t moveSpeed,
	cmmt_float_t viewSpeed,
	cmmt_float_t fieldOfView,
	cmmt_float_t nearClipPlane,
	cmmt_float_t farClipPlane,
	KeyboardKey moveLeftKey,
	KeyboardKey moveRightKey,
	KeyboardKey moveDownKey,
	KeyboardKey moveUpKey,
	KeyboardKey moveBackwardKey,
	KeyboardKey moveForwardKey)
{
	assert(framebuffer);
	assert(transformer);
	assert(fieldOfView > 0.0);
	assert(nearClipPlane < farClipPlane);

	FreeCamera freeCamera = calloc(1,
		sizeof(FreeCamera_T));

	if (!freeCamera)
		return NULL;

	freeCamera->framebuffer = framebuffer;
	freeCamera->rotation = zeroVec2F;
	freeCamera->lastCursorPosition = zeroVec2F;
	freeCamera->velocity = zeroVec3F;
	freeCamera->moveSpeed = moveSpeed;
	freeCamera->viewSpeed = viewSpeed;
	freeCamera->fieldOfView = fieldOfView;
	freeCamera->nearClipPlane = nearClipPlane;
	freeCamera->farClipPlane = farClipPlane;
	freeCamera->moveLeftKey = moveLeftKey;
	freeCamera->moveRightKey = moveRightKey;
	freeCamera->moveDownKey = moveDownKey;
	freeCamera->moveUpKey = moveUpKey;
	freeCamera->moveBackwardKey = moveBackwardKey;
	freeCamera->moveForwardKey = moveForwardKey;

	Transform transform = createTransform(
		transformer,
		zeroVec3F,
		oneVec3F,
		oneQuat,
		zeroVec3F,
		CAMERA_ROTATION_TYPE,
		NULL,
		freeCamera,
		true);

	if (!transform)
	{
		destroyFreeCamera(freeCamera);
		return NULL;
	}

	freeCamera->transform = transform;
	return freeCamera;
}
void destroyFreeCamera(FreeCamera freeCamera)
{
	if (!freeCamera)
		return;

	destroyTransform(freeCamera->transform);
	free(freeCamera);
}

Framebuffer getFreeCameraFramebuffer(FreeCamera freeCamera)
{
	assert(freeCamera);
	return freeCamera->framebuffer;
}
Transform getFreeCameraTransform(FreeCamera freeCamera)
{
	assert(freeCamera);
	return freeCamera->transform;
}
Vec3F getFreeCameraViewDirection(FreeCamera freeCamera)
{
	assert(freeCamera);
	Quat rotation = getTransformRotation(
		freeCamera->transform);
	return normVec3F(dotVecQuat3F(
		frontVec3F, rotation));
}

Vec3F getFreeCameraPosition(
	FreeCamera freeCamera)
{
	assert(freeCamera);

	return negVec3F(getTransformPosition(
		freeCamera->transform));
}
void setFreeCameraPosition(
	FreeCamera freeCamera,
	Vec3F position)
{
	assert(freeCamera);

	setTransformPosition(
		freeCamera->transform,
		negVec3F(position));
}

Vec2F getFreeCameraRotation(
	FreeCamera freeCamera)
{
	assert(freeCamera);
	return freeCamera->rotation;
}
void setFreeCameraRotation(
	FreeCamera freeCamera,
	Vec2F rotation)
{
	assert(freeCamera);

	if (rotation.y > degToRad((cmmt_float_t)89.99))
		rotation.y = degToRad((cmmt_float_t)89.99);
	else if (rotation.y < degToRad((cmmt_float_t)-89.99))
		rotation.y = degToRad((cmmt_float_t)-89.99);

	freeCamera->rotation = rotation;
}

cmmt_float_t getFreeCameraMoveSpeed(
	FreeCamera freeCamera)
{
	assert(freeCamera);
	return freeCamera->moveSpeed;
}
void setFreeCameraMoveSpeed(
	FreeCamera freeCamera,
	cmmt_float_t moveSpeed)
{
	assert(freeCamera);
	freeCamera->moveSpeed = moveSpeed;
}

cmmt_float_t getFreeCameraViewSpeed(
	FreeCamera freeCamera)
{
	assert(freeCamera);
	return freeCamera->viewSpeed;
}
void setFreeCameraViewSpeed(
	FreeCamera freeCamera,
	cmmt_float_t viewSpeed)
{
	assert(freeCamera);
	freeCamera->viewSpeed = viewSpeed;
}

cmmt_float_t getFreeCameraFieldOfView(
	FreeCamera freeCamera)
{
	assert(freeCamera);
	return freeCamera->fieldOfView;
}
void setFreeCameraFieldOfView(
	FreeCamera freeCamera,
	cmmt_float_t fieldOfView)
{
	assert(freeCamera);
	freeCamera->fieldOfView = fieldOfView;
}

cmmt_float_t getFreeCameraNearClipPlane(
	FreeCamera freeCamera)
{
	assert(freeCamera);
	return freeCamera->nearClipPlane;
}
void setFreeCameraNearClipPlane(
	FreeCamera freeCamera,
	cmmt_float_t nearClipPlane)
{
	assert(freeCamera);
	freeCamera->nearClipPlane = nearClipPlane;
}

cmmt_float_t getFreeCameraFarClipPlane(
	FreeCamera freeCamera)
{
	assert(freeCamera);
	return freeCamera->farClipPlane;
}
void setFreeCameraFarClipPlane(
	FreeCamera freeCamera,
	cmmt_float_t farClipPlane)
{
	assert(freeCamera);
	freeCamera->farClipPlane = farClipPlane;
}

KeyboardKey getFreeCameraMoveLeftKey(
	FreeCamera freeCamera)
{
	assert(freeCamera);
	return freeCamera->moveLeftKey;
}
void setFreeCameraMoveLeftKey(
	FreeCamera freeCamera,
	KeyboardKey moveLeftKey)
{
	assert(freeCamera);
	assert(moveLeftKey <= LAST_KEYBOARD_KEY);
	freeCamera->moveLeftKey = moveLeftKey;
}

KeyboardKey getFreeCameraMoveRightKey(
	FreeCamera freeCamera)
{
	assert(freeCamera);
	return freeCamera->moveRightKey;
}
void setFreeCameraMoveRightKey(
	FreeCamera freeCamera,
	KeyboardKey moveRightKey)
{
	assert(freeCamera);
	assert(moveRightKey <= LAST_KEYBOARD_KEY);
	freeCamera->moveRightKey = moveRightKey;
}

KeyboardKey getFreeCameraMoveDownKey(
	FreeCamera freeCamera)
{
	assert(freeCamera);
	return freeCamera->moveDownKey;
}
void setFreeCameraMoveDownKey(
	FreeCamera freeCamera,
	KeyboardKey moveDownKey)
{
	assert(freeCamera);
	assert(moveDownKey <= LAST_KEYBOARD_KEY);
	freeCamera->moveDownKey = moveDownKey;
}

KeyboardKey getFreeCameraMoveUpKey(
	FreeCamera freeCamera)
{
	assert(freeCamera);
	return freeCamera->moveUpKey;
}
void setFreeCameraMoveUpKey(
	FreeCamera freeCamera,
	KeyboardKey moveUpKey)
{
	assert(freeCamera);
	assert(moveUpKey <= LAST_KEYBOARD_KEY);
	freeCamera->moveUpKey = moveUpKey;
}

KeyboardKey getFreeCameraMoveBackwardKey(
	FreeCamera freeCamera)
{
	assert(freeCamera);
	return freeCamera->moveBackwardKey;
}
void setFreeCameraMoveBackwardKey(
	FreeCamera freeCamera,
	KeyboardKey moveBackwardKey)
{
	assert(freeCamera);
	assert(moveBackwardKey <= LAST_KEYBOARD_KEY);
	freeCamera->moveBackwardKey = moveBackwardKey;
}

KeyboardKey getFreeCameraMoveForwardKey(
	FreeCamera freeCamera)
{
	assert(freeCamera);
	return freeCamera->moveForwardKey;
}
void setFreeCameraMoveForwardKey(
	FreeCamera freeCamera,
	KeyboardKey moveForwardKey)
{
	assert(freeCamera);
	assert(moveForwardKey <= LAST_KEYBOARD_KEY);
	freeCamera->moveForwardKey = moveForwardKey;
}

void updateFreeCamera(FreeCamera freeCamera)
{
	assert(freeCamera);

	Window window = getFramebufferWindow(freeCamera->framebuffer);
	cmmt_float_t deltaTime = (cmmt_float_t)getWindowDeltaTime(window);

	if (!isWindowFocused(window))
		return;

	if (getWindowMouseButton(window, RIGHT_MOUSE_BUTTON))
	{
		setWindowCursorMode(window, LOCKED_CURSOR_MODE);
		Transform transform = freeCamera->transform;
		Vec2F rotation = freeCamera->rotation;
		Vec2F lastCursorPosition = freeCamera->lastCursorPosition;
		cmmt_float_t moveSpeed = freeCamera->moveSpeed * (cmmt_float_t)2.0;
		cmmt_float_t viewSpeed = freeCamera->viewSpeed * (cmmt_float_t)(1.0 / 180.0);
		Vec2F cursorPosition = getWindowCursorPosition(window);

		if (lastCursorPosition.x == 0 && lastCursorPosition.y == 0)
			lastCursorPosition = cursorPosition;

		rotation.x += (cursorPosition.y - lastCursorPosition.y) * viewSpeed;
		rotation.y += (cursorPosition.x - lastCursorPosition.x) * viewSpeed;

		rotation.x = clamp(rotation.x,
			degToRad((cmmt_float_t)-89.99),
			degToRad((cmmt_float_t)89.99));

		freeCamera->rotation = rotation;
		freeCamera->lastCursorPosition = cursorPosition;

		Quat transformRotation = axisQuat(rotation.x, leftVec3F);
		transformRotation = dotQuat(transformRotation,
			axisQuat(rotation.y, bottomVec3F));
		setTransformRotation(transform, transformRotation);

		Vec3F moveVector = zeroVec3F;

		if (getWindowKeyboardKey(window, freeCamera->moveLeftKey))
			moveVector.x = LEFT_AXIS_VALUE * deltaTime * moveSpeed;
		else if (getWindowKeyboardKey(window, freeCamera->moveRightKey))
			moveVector.x = RIGHT_AXIS_VALUE * deltaTime * moveSpeed;
		if (getWindowKeyboardKey(window, freeCamera->moveDownKey))
			moveVector.y = BOTTOM_AXIS_VALUE * deltaTime * moveSpeed;
		else if (getWindowKeyboardKey(window, freeCamera->moveUpKey))
			moveVector.y = TOP_AXIS_VALUE * deltaTime * moveSpeed;
		if (getWindowKeyboardKey(window, freeCamera->moveBackwardKey))
			moveVector.z = BACK_AXIS_VALUE * deltaTime * moveSpeed;
		else if (getWindowKeyboardKey(window, freeCamera->moveForwardKey))
			moveVector.z = FRONT_AXIS_VALUE * deltaTime * moveSpeed;

		Vec3F velocity = freeCamera->velocity;
		moveVector = dotVecQuat3F(moveVector, transformRotation);
		velocity = lerpValVec3F(velocity, moveVector, deltaTime * LERP_FACTOR);
		freeCamera->velocity = velocity;

		Vec3F transformPosition = getTransformPosition(transform);
		transformPosition = addVec3F(transformPosition, velocity);
		setTransformPosition(transform, transformPosition);
	}
	else
	{
		setWindowCursorMode(window, DEFAULT_CURSOR_MODE);
		freeCamera->lastCursorPosition = zeroVec2F;

		Transform transform = freeCamera->transform;
		Vec3F velocity = freeCamera->velocity;
		velocity = lerpValVec3F(velocity, zeroVec3F, deltaTime * LERP_FACTOR);
		freeCamera->velocity = velocity;

		Vec3F transformPosition = getTransformPosition(transform);
		transformPosition = addVec3F(transformPosition, velocity);
		setTransformPosition(transform, transformPosition);
	}
}
Camera getFreeCamera(FreeCamera freeCamera)
{
	assert(freeCamera);

	Vec2I framebufferSize = getFramebufferSize(
		freeCamera->framebuffer);

	return perspCamera(
		freeCamera->fieldOfView,
		(cmmt_float_t)framebufferSize.x /
			(cmmt_float_t)framebufferSize.y,
		freeCamera->nearClipPlane,
		freeCamera->farClipPlane);
}
