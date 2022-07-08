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
#include "uran/text.h"

#include "cmmt/camera.h"
#include "cmmt/bounding.h"
#include "mpmt/thread_pool.h"

#if _WIN32
#undef interface
#endif

/*
 * Interface structure.
 */
typedef struct Interface_T Interface_T;
/*
 * Interface instance.
 */
typedef Interface_T* Interface;

/*
 * Interface element structure.
 */
typedef struct InterfaceElement_T InterfaceElement_T;
/*
 * Interface element instance.
 */
typedef InterfaceElement_T* InterfaceElement;

/*
 * Interface element destroy function.
 * handle - handle instance or NULL.
 */
typedef void(*OnInterfaceElementDestroy)(
	void* handle);
/*
 * Interface element event function.
 * element - interface element instance.
 */
typedef void(*OnInterfaceElementEvent)(
	InterfaceElement element);

/*
 * Interface enumeration function.
 *
 * element - interface element instance.
 * handle - handle instance or NULL.
 */
typedef void(*OnInterfaceElement)(
	InterfaceElement element, void* handle);

/*
 * Interface elements events structure
 */
typedef struct InterfaceElementEvents
{
	OnInterfaceElementEvent onUpdate;
	OnInterfaceElementEvent onEnable;
	OnInterfaceElementEvent onDisable;
	OnInterfaceElementEvent onEnter;
	OnInterfaceElementEvent onExit;
	OnInterfaceElementEvent onStay;
	OnInterfaceElementEvent onPress;
	OnInterfaceElementEvent onRelease;
} InterfaceElementEvents;

/*
 * Empty interface element events.
 */
static const InterfaceElementEvents emptyInterfaceElementEvents = {
	NULL, NULL, NULL, NULL, NULL, NULL, NULL,
};

/*
 * Create a new interface instance.
 * Returns interface instance on success, otherwise NULL.
 *
 * window - window instance.
 * scale - interface scale multiplier.
 * capacity - initial interface element capacity.
 * threadPool - thread pool instance or NULL.
 */
Interface createInterface(
	Window window,
	cmmt_float_t scale,
	size_t capacity,
	ThreadPool threadPool);
/*
 * Destroys interface instance.
 * interface - interface instance or NULL.
 */
void destroyInterface(Interface interface);

/*
 * Returns interface window instance.
 * interface - interface instance.
 */
Window getInterfaceWindow(Interface interface);
/*
 * Returns interface thread pool instance.
 * interface - interface instance.
 */
ThreadPool getInterfaceThreadPool(Interface interface);
/*
 * Returns interface element count.
 * interface - interface instance.
 */
size_t getInterfaceElementCount(Interface interface);

/*
 * Returns interface scale multiplier value.
 * interface - interface instance.
 */
cmmt_float_t getInterfaceScale(
	Interface interface);
/*
 * Sets interface scale multiplier value.
 *
 * interface - interface instance.
 * scale - interface scale multiplier value.
 */
void setInterfaceScale(
	Interface interface,
	cmmt_float_t scale);

/*
 * Enumerates interface elements.
 *
 * interface - interface instance.
 * onElement - on interface element function.
 * handle - function argument or NULL.
 */
void enumerateInterfaceElements(
	Interface interface,
	OnInterfaceElement onElement,
	void* handle);
/*
 * Enumerates interface elements using thread pool.
 *
 * interface - interface instance.
 * onElement - on interface element function.
 * handle - function argument or NULL.
 */
void threadedEnumerateInterfaceElements(
	Interface interface,
	OnInterfaceElement onElement,
	void* handle);

/*
 * Destroys all interface elements.
 *
 * interface - transformer instance.
 * destroyTransforms - destroy also transform instances.
 */
void destroyAllInterfaceElements(
	Interface interface,
	bool destroyTransforms);

/*
 * Creates interface camera.
 * interface - interface instance.
 */
Camera createInterfaceCamera(Interface interface);
/*
 * Returns interface relative cursor position.
 * interface - interface instance.
 */
Vec2F getInterfaceCursorPosition(Interface interface);

/*
 * Processes interface events and bakes data.
 * interface - interface instance.
 */
void updateInterface(Interface interface);

/*
 * Create a new interface element instance.
 * Returns interface element instance on success, otherwise NUL.
 *
 * interface - interface instance.
 * transform - transform instance.
 * alignment - interface element alignment type.
 * position - interface element position.
 * bounds - interface element bounds.
 * isEnabled - is interface element enabled.
 * onDestroy - element destroy function.
 * events - interface element events or NULL.
 * handle - interface element handle.
 */
InterfaceElement createInterfaceElement(
	Interface interface,
	Transform transform,
	AlignmentType alignment,
	Vec3F position,
	Box2F bounds,
	bool isEnabled,
	OnInterfaceElementDestroy onDestroy,
	const InterfaceElementEvents* events,
	void* handle);
/*
 * Destroy interface element instance.
 *
 * element - interface element instance or NULL.
 * destroyTransform - destroy also transform instance.
 */
void destroyInterfaceElement(InterfaceElement element);

/*
 * Returns interface element interface.
 * element - interface element instance.
 */
Interface getInterfaceElementInterface(InterfaceElement element);
/*
 * Returns interface element transform.
 * element - interface element instance.
 */
Transform getInterfaceElementTransform(InterfaceElement element);
/*
 * Returns interface element on destroy function.
 * element - interface element instance.
 */
OnInterfaceElementDestroy getInterfaceElementOnDestroy(InterfaceElement element);
/*
 * Returns interface element events.
 * element - interface element instance.
 */
const InterfaceElementEvents* getInterfaceElementEvents(InterfaceElement element);
/*
 * Returns interface element handel.
 * element - interface element instance.
 */
void* getInterfaceElementHandle(InterfaceElement element);

/*
 * Returns interface element alignment.
 * element - interface element instance.
 */
AlignmentType getInterfaceElementAlignment(
	InterfaceElement element);
/*
 * Sets interface element alignment.
 *
 * element - interface element instance.
 * alignment - interface element alignment type.
 */
void setInterfaceElementAlignment(
	InterfaceElement element,
	AlignmentType alignment);

/*
 * Returns interface element position.
 * element - interface element instance.
 */
Vec3F getInterfaceElementPosition(
	InterfaceElement element);
/*
 * Sets interface element position.
 *
 * element - interface element instance.
 * position - interface element position.
 */
void setInterfaceElementPosition(
	InterfaceElement element,
	Vec3F position);

/*
 * Returns interface element bounds.
 * element - interface element instance.
 */
Box2F getInterfaceElementBounds(
	InterfaceElement element);
/*
 * Sets interface element bounds.
 *
 * element - interface element instance.
 * position - interface element bounds.
 */
void setInterfaceElementBounds(
	InterfaceElement element,
	Box2F bounds);

/*
 * Returns true if interface element is enabled.
 * element - interface element instance.
 */
bool isInterfaceElementEnabled(
	InterfaceElement element);
/*
 * Sets interface element enabled value.
 *
 * element - interface element instance.
 * isEnabled - is interface element enabled.
 */
void setInterfaceElementEnabled(
	InterfaceElement element,
	bool isEnabled);

/*
 * Bake specific interface element.
 * element - interface element instance.
 */
void bakeInterfaceElement(InterfaceElement element);
