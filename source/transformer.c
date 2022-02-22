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
	atomic_int64 threadIndex;
#ifndef NDEBUG
	bool isEnumerating;
#endif
};
struct Transform_T
{
	Transformer transformer;
	Transform parent;
	Mat4F model;
	Quat rotation;
	Vec3F scale;
	Vec3F position;
	RotationType rotationType;
	bool isActive;
};

inline static void updateTransformModel(
	Transform transform,
	Vec3F cameraPosition)
{
	Vec3F position = transform->position;
	Quat rotation = transform->rotation;
	Transform parent = transform->parent;

	position = addVec3F(position, cameraPosition);

	while (parent)
	{
		if (!parent->isActive)
			return;

		rotation = normQuat(dotQuat(
			rotation, parent->rotation));
		position = addVec3F(parent->position,
			dotQuatVec3F(rotation, position));
		parent = parent->parent;
	}

	RotationType rotationType = transform->rotationType;

	Mat4F model;

	if (rotationType == SPIN_ROTATION_TYPE)
	{
		model = dotMat4F(translateMat4F(
			identMat4F, position),
			getQuatMat4F(normQuat(rotation)));
	}
	else if (rotationType == ORBIT_ROTATION_TYPE)
	{
		model = translateMat4F(dotMat4F(
			identMat4F, getQuatMat4F(
			normQuat(rotation))),position);
	}
	else
	{
		model = translateMat4F(identMat4F,position);
	}

	transform->model = scaleMat4F(model, transform->scale);
}

Transformer createTransformer(
	size_t capacity,
	ThreadPool threadPool)
{
	Transformer transformer = calloc(1,
		sizeof(Transformer_T));

	if (!transformer)
		return NULL;

	transformer->threadPool = threadPool;
	transformer->camera = NULL;
	transformer->threadIndex = 0;
#ifndef NDEBUG
	transformer->isEnumerating = false;
#endif

	if (capacity == 0)
		capacity = MPGX_DEFAULT_CAPACITY;

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
size_t getTransformerTransformCount(Transformer transformer)
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

void enumerateTransformer(
	Transformer transformer,
	OnTransformItem onItem,
	void* functionArgument)
{
	assert(transformer);
	assert(onItem);

#ifndef NDEBUG
	transformer->isEnumerating = true;
#endif

	Transform* transforms = transformer->transforms;
	size_t transformCount = transformer->transformCount;

	for (size_t i = 0; i < transformCount; i++)
		onItem(transforms[i], functionArgument);

#ifndef NDEBUG
	transformer->isEnumerating = false;
#endif
}
void destroyAllTransformerTransforms(Transformer transformer)
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

static void onTransformUpdate(void* argument)
{
	Transformer transformer = argument;
	Transform* transforms = transformer->transforms;
	size_t transformCount = transformer->transformCount;
	Transform cameraTransform = transformer->camera;

	Vec3F cameraPosition = cameraTransform ?
		cameraTransform->position : zeroVec3F;

	size_t threadCount = getThreadPoolThreadCount(
		transformer->threadPool);
	atomic_int64 threadIndex = atomicFetchAdd64(
		&transformer->threadIndex, 1);

	for (size_t i = threadIndex; i < transformCount; i += threadCount)
	{
		Transform transform = transforms[i];

		if (!transform->isActive)
			continue;

		updateTransformModel(
			transform,
			cameraPosition);
	}

	if (cameraTransform)
	{
		updateTransformModel(
			cameraTransform,
			negVec3F(cameraPosition));
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

	if (threadPool && transformCount > getThreadPoolThreadCount(threadPool))
	{
		waitThreadPool(threadPool);
		transformer->threadIndex = 0;

		size_t threadCount = getThreadPoolThreadCount(threadPool);

		bool result = true;

		for (size_t i = 0; i < threadCount; i++)
		{
			result &= addThreadPoolTask(
				threadPool,
				onTransformUpdate,
				transformer);
		}

		if (!result)
			abort();

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
				cameraPosition);
		}

		if (cameraTransform)
		{
			updateTransformModel(
				cameraTransform,
				negVec3F(cameraPosition));
		}
	}
}

Transform createTransform(
	Transformer transformer,
	Vec3F position,
	Vec3F scale,
	Quat rotation,
	RotationType rotationType,
	Transform parent,
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
	transform->parent = parent;
	transform->rotation = rotation;
	transform->scale = scale;
	transform->position = position;
	transform->rotationType = rotationType;
	transform->isActive = isActive;

	Transform cameraTransform = transformer->camera;

	Vec3F cameraPosition = cameraTransform ?
		cameraTransform->position : zeroVec3F;

	updateTransformModel(
		transform,
		cameraPosition);

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
Transform createDefaultTransform(Transformer transformer)
{
	assert(transformer);
	assert(!transformer->isEnumerating);

	return createTransform(
		transformer,
		zeroVec3F,
		oneVec3F,
		oneQuat,
		SPIN_ROTATION_TYPE,
		NULL,
		true);
}
void destroyTransform(Transform transform)
{
	if (!transform)
		return;

	assert(!transform->transformer->isEnumerating);

	Transformer transformer = transform->transformer;
	Transform* transforms = transformer->transforms;
	size_t transformCount = transformer->transformCount;

	for (size_t i = 0; i < transformCount; i++)
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
	transform->parent = parent;
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
