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
 */
typedef void(*OnInterfaceElementDestroy)(
	void* handle);
/*
 * Interface element event function.
 */
typedef void(*OnInterfaceElementEvent)(
	InterfaceElement element);

/*
 * Interface element enumeration function.
 */
typedef void(*OnInterfaceElement)(
	InterfaceElement interfaceElement, void* handle);

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
 * Create a new interface instance.
 * Returns interface instance on success, otherwise NULL.
 *
 * window - window instance.
 * scale - interface scale multiplier.
 * capacity - initial interface element capacity.
 */
Interface createInterface(
	Window window,
	float scale,
	size_t capacity);
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
 * Returns interface element count.
 * interface - interface instance.
 */
size_t getInterfaceElementCount(Interface interface);

/*
 * Returns interface scale multiplier value.
 * interface - interface instance.
 */
float getInterfaceScale(
	Interface interface);
/*
 * Sets interface scale multiplier value.
 *
 * interface - interface instance.
 * scale - interface scale multiplier value.
 */
void setInterfaceScale(
	Interface interface,
	float scale);

/*
 * Enumerates interface elements.
 *
 * interface - interface instance.
 * onElement - on element function.
 * functionArgument - function argument or NULL.
 */
void enumerateInterface(
	Interface interface,
	OnInterfaceElement onElement,
	void* functionArgument);
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
 * Bakes interface elements.
 * interface - interface instance.
 */
void preUpdateInterface(Interface interface);
/*
 * Processes interface events.
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
 * onDestroy - on element destroy function.
 * events - interface element events.
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
 * Create a new interface element instance with default values.
 * Returns interface element instance on success, otherwise NUL.
 *
 * interface - interface instance.
 * transform - transform instance.
 * onDestroy - on element destroy function.
 * events - interface element events.
 * handle - interface element handle.
 */
InterfaceElement createDefaultInterfaceElement(
	Interface interface,
	Transform transform,
	OnInterfaceElementDestroy onDestroy,
	const InterfaceElementEvents* events,
	void* handle);
/*
 * Destroy interface element instance.
 *
 * element - interface element instance or NULL.
 * destroyTransform - destroy also transform instance.
 */
void destroyInterfaceElement(
	InterfaceElement element,
	bool destroyTransform);

/*
 * Returns interface element interface.
 * element - interface element instance.
 */
Interface getInterfaceElementInterface(
	InterfaceElement element);
/*
 * Returns interface element transform.
 * element - interface element instance.
 */
Transform getInterfaceElementTransform(
	InterfaceElement element);
/*
 * Returns interface element on destroy function.
 * element - interface element instance.
 */
OnInterfaceElementDestroy getInterfaceElementOnDestroy(
	InterfaceElement element);
/*
 * Returns interface element events.
 * element - interface element instance.
 */
const InterfaceElementEvents* getInterfaceElementEvents(
	InterfaceElement element);
/*
 * Returns interface element handel.
 * element - interface element instance.
 */
void* getInterfaceElementHandle(
	InterfaceElement element);

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
