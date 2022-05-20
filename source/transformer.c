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

#include "uran/transformer.h"

#include "mpgx/defines.h"
#include "mpmt/atomic.h"
#include "cmmt/matrix.h"

#include <stdlib.h>
#include <assert.h>

struct Transformer_T
{
	ThreadPool threadPool;
	Transform* transforms;
	size_t transformCapacity;
	size_t transformCount;
	Transform camera;
#ifndef NDEBUG
	bool isEnumerating;
#endif
};
struct Transform_T
{
	Transformer transformer;
	void* handle;
	Transform parent;
	Mat4F model;
	Quat rotation;
	Vec3F scale;
	Vec3F position;
	Vec3F pivot;
	RotationType rotationType;
	bool isActive;
};

inline static void updateTransformModel(
	Transform transform,
	Vec3F cameraPosition,
	bool forceUpdate)
{
	assert(transform);

	Vec3F position = transform->position;
	Quat rotation = transform->rotation;
	Transform parent = transform->parent;

	position = subVec3F(position, cameraPosition);

	if (forceUpdate)
	{
		while (parent)
		{
			position = addVec3F(dotQuatVec3F(
				parent->rotation, position),
				parent->position);
			rotation = dotQuat(rotation, parent->rotation);
			parent = parent->parent;
		}
	}
	else
	{
		while (parent)
		{
			if (!parent->isActive)
				return;

			position = addVec3F(dotQuatVec3F(
				parent->rotation, position),
				parent->position);
			rotation = dotQuat(rotation, parent->rotation);
			parent = parent->parent;
		}
	}

	RotationType rotationType = transform->rotationType;

	Mat4F model;

	if (rotationType == SPIN_ROTATION_TYPE)
	{
		model = dotMat4F(translateMat4F(
			identMat4F, position),
			getQuatMat4F(normQuat(rotation)));
	}
	else if (rotationType == CAMERA_ROTATION_TYPE)
	{
		model = translateMat4F(getQuatMat4F(
			normQuat(rotation)),
			negVec3F(position));
	}
	else
	{
		model = translateMat4F(identMat4F,position);
	}

	transform->model = translateMat4F(scaleMat4F(
		model, transform->scale),
		negVec3F(transform->pivot));
}

Transformer createTransformer(
	size_t capacity,
	ThreadPool threadPool)
{
	assert(capacity > 0);

	Transformer transformer = calloc(1,
		sizeof(Transformer_T));

	if (!transformer)
		return NULL;

	transformer->threadPool = threadPool;
	transformer->camera = NULL;
#ifndef NDEBUG
	transformer->isEnumerating = false;
#endif

	Transform* transforms = malloc(
		sizeof(Transform) * capacity);

	if (!transforms)
	{
		destroyTransformer(transformer);
		return NULL;
	}

	transformer->transforms = transforms;
	transformer->transformCapacity = capacity;
	transformer->transformCount = 0;
	return transformer;
}
void destroyTransformer(Transformer transformer)
{
	if (!transformer)
		return;

	assert(transformer->transformCount == 0);
	assert(!transformer->isEnumerating);

	free(transformer->transforms);
	free(transformer);
}

ThreadPool getTransformerThreadPool(Transformer transformer)
{
	assert(transformer);
	return transformer->threadPool;
}
size_t getTransformCount(Transformer transformer)
{
	assert(transformer);
	return transformer->transformCount;
}

Transform getTransformerCamera(
	Transformer transformer)
{
	assert(transformer);
	return transformer->camera;
}
void setTransformerCamera(
	Transformer transformer,
	Transform camera)
{
	assert(transformer);
	transformer->camera = camera;
}

void enumerateTransformerItems(
	Transformer transformer,
	OnTransformerItem onItem,
	void* handle)
{
	assert(transformer);
	assert(onItem);

#ifndef NDEBUG
	transformer->isEnumerating = true;
#endif

	Transform* transforms = transformer->transforms;
	size_t transformCount = transformer->transformCount;

	for (size_t i = 0; i < transformCount; i++)
		onItem(transforms[i], handle);

#ifndef NDEBUG
	transformer->isEnumerating = false;
#endif
}

typedef struct EnumerateData
{
	Transformer transformer;
	OnTransformerItem onItem;
	void* handle;
	atomic_int64 threadIndex;
} EnumerateData;
static void onTransformerEnumerate(void* argument)
{
	assert(argument);
	EnumerateData* data = (EnumerateData*)argument;
	Transformer transformer = data->transformer;
	OnTransformerItem onItem = data->onItem;
	void* handle = data->handle;
	Transform* transforms = transformer->transforms;
	size_t transformCount = transformer->transformCount;

	size_t threadCount = getThreadPoolThreadCount(
		transformer->threadPool);
	size_t threadIndex = (size_t)atomicFetchAdd64(
		&data->threadIndex, 1);

	for (size_t i = threadIndex; i < transformCount; i += threadCount)
		onItem(transforms[i], handle);
}
void threadedEnumerateTransformerItems(
	Transformer transformer,
	OnTransformerItem onItem,
	void* handle)
{
	assert(transformer);
	assert(onItem);
	assert(transformer->threadPool);

	size_t transformCount = transformer->transformCount;

	if (!transformCount)
		return;

#ifndef NDEBUG
	transformer->isEnumerating = true;
#endif

	if (transformCount >= getThreadPoolThreadCount(transformer->threadPool))
	{
		ThreadPool threadPool = transformer->threadPool;
		size_t threadCount = getThreadPoolThreadCount(threadPool);

		EnumerateData data = {
			transformer,
			onItem,
			handle,
			0,
		};
		ThreadPoolTask task = {
			onTransformerEnumerate,
			&data
		};
		addThreadPoolTaskNumber(
			threadPool,
			task,
			threadCount);
		waitThreadPool(threadPool);
	}
	else
	{
		enumerateTransformerItems(
			transformer,
			onItem,
			handle);
	}

#ifndef NDEBUG
	transformer->isEnumerating = false;
#endif
}

void destroyAllTransformerItems(Transformer transformer)
{
	assert(transformer);
	assert(!transformer->isEnumerating);

	Transform* transforms = transformer->transforms;
	size_t transformCount = transformer->transformCount;

	if (transformCount == 0)
		return;

	for (size_t i = 0; i < transformCount; i++)
		free(transforms[i]);

	transformer->transformCount = 0;
}

typedef struct UpdateData
{
	Transformer transformer;
	atomic_int64 threadIndex;
} UpdateData;
static void onTransformUpdate(void* argument)
{
	assert(argument);

	UpdateData* data = (UpdateData*)argument;
	Transformer transformer = data->transformer;
	Transform* transforms = transformer->transforms;
	size_t transformCount = transformer->transformCount;
	Transform cameraTransform = transformer->camera;

	Vec3F cameraPosition = cameraTransform ?
		cameraTransform->position : zeroVec3F;

	size_t threadCount = getThreadPoolThreadCount(
		transformer->threadPool);
	size_t threadIndex = (size_t)atomicFetchAdd64(
		&data->threadIndex, 1);

	for (size_t i = threadIndex; i < transformCount; i += threadCount)
	{
		Transform transform = transforms[i];

		if (!transform->isActive)
			continue;

		updateTransformModel(
			transform,
			cameraPosition,
			false);
	}
}
void updateTransformer(Transformer transformer)
{
	assert(transformer);
	assert(!transformer->isEnumerating);

	size_t transformCount = transformer->transformCount;

	if (!transformCount)
		return;

	ThreadPool threadPool = transformer->threadPool;

	if (threadPool && transformCount >= getThreadPoolThreadCount(threadPool))
	{
		size_t threadCount = getThreadPoolThreadCount(threadPool);

		UpdateData data = {
			transformer,
			0,
		};
		ThreadPoolTask task = {
			onTransformUpdate,
			&data,
		};
		addThreadPoolTaskNumber(
			threadPool,
			task,
			threadCount);
		waitThreadPool(threadPool);
	}
	else
	{
		Transform* transforms = transformer->transforms;
		Transform cameraTransform = transformer->camera;

		Vec3F cameraPosition = cameraTransform ?
			cameraTransform->position : zeroVec3F;

		for (size_t i = 0; i < transformCount; i++)
		{
			Transform transform = transforms[i];

			if (!transform->isActive)
				continue;

			updateTransformModel(
				transform,
				cameraPosition,
				false);
		}
	}
}

Transform createTransform(
	Transformer transformer,
	Vec3F position,
	Vec3F scale,
	Quat rotation,
	Vec3F pivot,
	RotationType rotationType,
	Transform parent,
	void* handle,
	bool isActive)
{
	assert(transformer);
	assert(rotationType < ROTATION_TYPE_COUNT);
	assert(!parent || (parent &&
		transformer == parent->transformer));
	assert(!transformer->isEnumerating);

	Transform transform = malloc(sizeof(Transform_T));

	if (!transform)
		return NULL;

	transform->transformer = transformer;
	transform->handle = handle;
	transform->parent = parent;
	transform->rotation = rotation;
	transform->scale = scale;
	transform->position = position;
	transform->pivot = pivot;
	transform->rotationType = rotationType;
	transform->isActive = isActive;

	Transform cameraTransform = transformer->camera;

	Vec3F cameraPosition = cameraTransform ?
		cameraTransform->position : zeroVec3F;

	updateTransformModel(
		transform,
		cameraPosition,
		true);

	size_t count = transformer->transformCount;

	if (count == transformer->transformCapacity)
	{
		size_t capacity = transformer->transformCapacity * 2;

		Transform* transforms = realloc(
			transformer->transforms,
			sizeof(Transform) * capacity);

		if (!transforms)
		{
			free(transform);
			return NULL;
		}

		transformer->transforms = transforms;
		transformer->transformCapacity = capacity;
	}

	transformer->transforms[count] = transform;
	transformer->transformCount = count + 1;
	return transform;
}
void destroyTransform(Transform transform)
{
	if (!transform)
		return;

	assert(!transform->transformer->isEnumerating);

	Transformer transformer = transform->transformer;
	Transform* transforms = transformer->transforms;
	size_t transformCount = transformer->transformCount;

	for (int64_t i = (int64_t)transformCount - 1; i >= 0; i--)
	{
		if (transforms[i] != transform)
			continue;

		for (size_t j = i + 1; j < transformCount; j++)
			transforms[j - 1] = transforms[j];

		free(transform);
		transformer->transformCount--;
		return;
	}

	abort();
}

Transformer getTransformTransformer(Transform transform)
{
	assert(transform);
	return transform->transformer;
}

Vec3F getTransformPosition(
	Transform transform)
{
	assert(transform);
	return transform->position;
}
void setTransformPosition(
	Transform transform,
	Vec3F position)
{
	assert(transform);
	transform->position = position;
}

Vec3F getTransformScale(
	Transform transform)
{
	assert(transform);
	return transform->scale;
}
void setTransformScale(
	Transform transform,
	Vec3F scale)
{
	assert(transform);
	transform->scale = scale;
}

Quat getTransformRotation(
	Transform transform)
{
	assert(transform);
	return transform->rotation;
}
void setTransformRotation(
	Transform transform,
	Quat rotation)
{
	assert(transform);
	transform->rotation = rotation;
}

Vec3F getTransformEulerAngles(
	Transform transform)
{
	assert(transform);
	return getQuatEuler(transform->rotation);
}
void setTransformEulerAngles(
	Transform transform,
	Vec3F eulerAngles)
{
	assert(transform);
	transform->rotation = eulerQuat(eulerAngles);
}

Vec3F getTransformPivot(
	Transform transform)
{
	assert(transform);
	return transform->pivot;
}
void setTransformPivot(
	Transform transform,
	Vec3F pivot)
{
	assert(transform);
	transform->pivot = pivot;
}

RotationType getTransformRotationType(
	Transform transform)
{
	assert(transform);
	return transform->rotationType;
}
void setTransformRotationType(
	Transform transform,
	RotationType rotationType)
{
	assert(transform);
	assert(rotationType < ROTATION_TYPE_COUNT);
	transform->rotationType = rotationType;
}

Transform getTransformParent(
	Transform transform)
{
	assert(transform);
	return transform->parent;
}
void setTransformParent(
	Transform transform,
	Transform parent)
{
	assert(!parent || (parent &&
		transform->transformer ==
		parent->transformer));
	assert(!parent || (parent != transform));
	transform->parent = parent;
}

void* getTransformHandle(
	Transform transform)
{
	assert(transform);
	return transform->handle;
}
void setTransformHandle(
	Transform transform,
	void* handle)
{
	assert(transform);
	transform->handle = handle;
}

bool isTransformActive(
	Transform transform)
{
	assert(transform);
	return transform->isActive;
}
void setTransformActive(
	Transform transform,
	bool isActive)
{
	assert(transform);
	transform->isActive = isActive;
}

Mat4F getTransformModel(Transform transform)
{
	assert(transform);
	return transform->model;
}
void bakeTransform(Transform transform)
{
	assert(transform);

	Transform cameraTransform =
		transform->transformer->camera;
	Vec3F cameraPosition = cameraTransform ?
		cameraTransform->position : zeroVec3F;

	updateTransformModel(
		transform,
		cameraPosition,
		true);
}
