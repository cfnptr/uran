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
#include "cmmt/quaternion.h"
#include "mpmt/thread_pool.h"

/*
 * Transformer structure.
 */
typedef struct Transformer_T Transformer_T;
/*
 * Transformer instance.
 */
typedef Transformer_T* Transformer;

/*
 * Transform structure.
 */
typedef struct Transform_T Transform_T;
/*
 * Transform instance.
 */
typedef Transform_T* Transform;

/*
 * Rotation types
 */
typedef enum RotationType_T
{
	NO_ROTATION_TYPE = 0,
	SPIN_ROTATION_TYPE = 1,
	CAMERA_ROTATION_TYPE = 2,
	ROTATION_TYPE_COUNT = 3,
} RotationType_T;
/*
 * Rotation type.
 */
typedef uint8_t RotationType;

/*
 * Transformer enumeration function.
 */
typedef void(*OnTransformerItem)(
	Transform transform, void* handle);

/*
 * Create a new transformer instance.
 * Returns transformer instance on success, otherwise NULL.
 *
 * capacity - initial transform array capacity.
 * threadPool - thread pool instance or NULL.
 */
Transformer createTransformer(
	size_t capacity,
	ThreadPool threadPool);
/*
 * Destroy transformer instance.
 * transformer - transformer instance or NULL.
 */
void destroyTransformer(Transformer transformer);

/*
 * Returns transformer thread pool instance.
 * transformer - transformer instance.
 */
ThreadPool getTransformerThreadPool(Transformer transformer);
/*
 * Returns transformer transform count.
 * transformer - transformer instance.
 */
size_t getTransformCount(Transformer transformer);

/*
 * Returns transformer camera transform.
 * transformer - transformer instance.
 */
Transform getTransformerCamera(
	Transformer transformer);
/*
 * Set transformer camera transform.
 * (Fixes precision on far distances)
 *
 * transformer - transformer instance.
 * camera - camera transform instance or NULL.
 */
void setTransformerCamera(
	Transformer transformer,
	Transform camera);

/*
 * Enumerates transformer transforms.
 *
 * transformer - transformer instance.
 * onItem - on transformer item function.
 * handle - function argument or NULL.
 */
void enumerateTransformerItems(
	Transformer transformer,
	OnTransformerItem onItem,
	void* handle);
/*
 * Enumerates transformer transforms using thread pool.
 *
 * transformer - transformer instance.
 * onItem - on transformer item function.
 * handle - function argument or NULL.
 */
void threadedEnumerateTransformerItems(
	Transformer transformer,
	OnTransformerItem onItem,
	void* handle);

/*
 * Destroys all transformer transforms.
 * transformer - transformer instance.
 */
void destroyAllTransformerItems(Transformer transformer);

/*
 * Bakes transformer transforms.
 * transformer - transformer instance.
 */
void updateTransformer(Transformer transformer);

/*
 * Create a new default transform instance.
 * Returns transform instance on success, otherwise NULL.
 *
 * transformer - transformer instance.
 * position - transform position.
 * scale - transform scale.
 * rotation - transform rotation.
 * pivot - transform pivot.
 * rotationType - transform rotation type.
 * parent - transform parent or NULL.
 * handle - custom handle or NULL.
 * isActive - is transform active.
 */
Transform createTransform(
	Transformer transformer,
	Vec3F position,
	Vec3F scale,
	Quat rotation,
	Vec3F pivot,
	RotationType rotationType,
	Transform parent,
	void* handle,
	bool isActive);
/*
 * Destroys transform instance.
 * transform - transform instance or NULL.
 */
void destroyTransform(Transform transform);

/*
 * Returns transform transformer.
 * transform - transform instance.
 */
Transformer getTransformTransformer(Transform transform);

/*
 * Returns transform position.
 * transform - transform instance.
 */
Vec3F getTransformPosition(
	Transform transform);
/*
 * Sets transform position.
 *
 * transform - transform instance.
 * position - transform position.
 */
void setTransformPosition(
	Transform transform,
	Vec3F position);

/*
 * Returns transform scale.
 * transform - transform instance.
 */
Vec3F getTransformScale(
	Transform transform);
/*
 * Sets transform scale.
 *
 * transform - transform instance.
 * scale - transform scale.
 */
void setTransformScale(
	Transform transform,
	Vec3F scale);

/*
 * Returns transform rotation.
 * transform - transform instance.
 */
Quat getTransformRotation(
	Transform transform);
/*
 * Sets transform rotation.
 *
 * transform - transform instance.
 * rotation - transform rotation.
 */
void setTransformRotation(
	Transform transform,
	Quat rotation);

/*
 * Returns transform euler angles.
 * transform - transform instance.
 */
Vec3F getTransformEulerAngles(
	Transform transform);
/*
 * Sets transform euler angles.
 *
 * transform - transform instance.
 * eulerAngles - transform euler angles.
 */
void setTransformEulerAngles(
	Transform transform,
	Vec3F eulerAngles);

/*
 * Returns transform pivot.
 * transform - transform instance.
 */
Vec3F getTransformPivot(
	Transform transform);
/*
 * Sets transform pivot.
 *
 * transform - transform instance.
 * scale - transform scale.
 */
void setTransformPivot(
	Transform transform,
	Vec3F pivot);

/*
 * Returns transform rotation type.
 * transform - transform instance.
 */
RotationType getTransformRotationType(
	Transform transform);
/*
 * Sets transform rotation type.
 *
 * transform - transform instance.
 * rotationType - transform rotation type.
 */
void setTransformRotationType(
	Transform transform,
	RotationType rotationType);

/*
 * Returns transform parent.
 * transform - transform instance.
 */
Transform getTransformParent(
	Transform transform);
/*
 * Sets transform parent.
 *
 * transform - transform instance.
 * parent - transform parent.
 */
void setTransformParent(
	Transform transform,
	Transform parent);

/*
 * Returns transform handle. (Use with caution!)
 * transform - transform instance.
 */
void* getTransformHandle(
	Transform transform);
/*
 * Sets transform handle. (Use with caution!)
 *
 * transform - transform instance.
 * parent - transform parent.
 */
void setTransformHandle(
	Transform transform,
	void* handle);

/*
 * Returns true if transform is active.
 * transform - transform instance.
 */
bool isTransformActive(
	Transform transform);
/*
* Sets transform is active.
*
* transform - transform instance.
* isActive - is transform active.
*/
void setTransformActive(
	Transform transform,
	bool isActive);

/*
 * Returns transform model matrix.
 * transform - transform instance.
 */
Mat4F getTransformModel(Transform transform);
/*
 * Bake specific transform.
 * transform - transform instance.
 */
void bakeTransform(Transform transform);
