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

struct FreeCamera_T
{
	Framebuffer framebuffer;
	Transform transform;
	Vec2F rotation;
	Vec2F lastCursorPosition;
	cmmt_float_t moveSpeed;
	cmmt_float_t viewSpeed;
	cmmt_float_t fieldOfView;
	cmmt_float_t nearClipPlane;
	cmmt_float_t farClipPlane;
};

FreeCamera createFreeCamera(
	Framebuffer framebuffer,
	Transformer transformer,
	cmmt_float_t moveSpeed,
	cmmt_float_t viewSpeed,
	cmmt_float_t fieldOfView,
	cmmt_float_t nearClipPlane,
	cmmt_float_t farClipPlane)
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
	freeCamera->moveSpeed = moveSpeed;
	freeCamera->viewSpeed = viewSpeed;
	freeCamera->fieldOfView = fieldOfView;
	freeCamera->nearClipPlane = nearClipPlane;
	freeCamera->farClipPlane = farClipPlane;

	Transform transform = createTransform(
		transformer,
		zeroVec3F,
		oneVec3F,
		oneQuat,
		ORBIT_ROTATION_TYPE,
		NULL,
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

Framebuffer getFreeCameraFramebuffer(
	FreeCamera freeCamera)
{
	assert(freeCamera);
	return freeCamera->framebuffer;
}
Vec3F getFreeCameraViewDirection(
	FreeCamera freeCamera)
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

void updateFreeCamera(FreeCamera freeCamera)
{
	assert(freeCamera);

	Window window = getFramebufferWindow(
		freeCamera->framebuffer);

	if (!isWindowFocused(window))
		return;

	if (getWindowMouseButton(window, RIGHT_MOUSE_BUTTON))
	{
		setWindowCursorMode(
			window,
			LOCKED_CURSOR_MODE);

		cmmt_float_t deltaTime = (cmmt_float_t)getWindowDeltaTime(window);
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

		Quat transformRotation = axisQuat(
			rotation.x, leftVec3F);
		transformRotation = dotQuat(
			transformRotation,
			axisQuat(rotation.y, bottomVec3F));
		setTransformRotation(
			transform,
			transformRotation);

		Vec3F translation = zeroVec3F;

		if (getWindowKeyboardKey(window, A_KEYBOARD_KEY))
			translation.x = RIGHT_AXIS_VALUE * deltaTime * moveSpeed;
		else if (getWindowKeyboardKey(window, D_KEYBOARD_KEY))
			translation.x = LEFT_AXIS_VALUE * deltaTime * moveSpeed;
		if (getWindowKeyboardKey(window, LEFT_SHIFT_KEYBOARD_KEY))
			translation.y = TOP_AXIS_VALUE * deltaTime * moveSpeed;
		else if (getWindowKeyboardKey(window, SPACE_KEYBOARD_KEY))
			translation.y = BOTTOM_AXIS_VALUE * deltaTime * moveSpeed;
		if (getWindowKeyboardKey(window, S_KEYBOARD_KEY))
			translation.z = FRONT_AXIS_VALUE * deltaTime * moveSpeed;
		else if (getWindowKeyboardKey(window, W_KEYBOARD_KEY))
			translation.z = BACK_AXIS_VALUE * deltaTime * moveSpeed;

		translation = dotVecQuat3F(
			translation,
			transformRotation);

		Vec3F transformPosition =
			getTransformPosition(transform);
		transformPosition = addVec3F(
			transformPosition,
			translation);
		setTransformPosition(
			transform,
			transformPosition);
	}
	else
	{
		setWindowCursorMode(
			window,
			DEFAULT_CURSOR_MODE);
		freeCamera->lastCursorPosition = zeroVec2F;
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
