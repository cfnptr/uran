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

#include "uran/interface.h"

#include <assert.h>
#include <stdlib.h>

struct InterfaceElement_T
{
	Interface interface;
	OnInterfaceElementDestroy onDestroy;
	InterfaceElementEvents events;
	void* handle;
	Transform transform;
	Vec3F position;
	Box2F bounds;
	AlignmentType alignment;
	bool isEnabled;
	bool isPressed;
#ifndef NDEBUG
	uint8_t _alignment[1];
	const char* name;
#endif
};
struct Interface_T
{
	Window window;
	InterfaceElement* elements;
	size_t elementCapacity;
	size_t elementCount;
	InterfaceElement lastElement;
	cmmt_float_t scale;
	bool isPressed;
#ifndef NDEBUG
	bool isEnumerating;
#endif
};

void destroyInterface(Interface interface)
{
	if (!interface)
		return;

	assert(interface->elementCount == 0);
	assert(!interface->isEnumerating);

	free(interface->elements);
	free(interface);
}
Interface createInterface(
	Window window,
	cmmt_float_t scale,
	size_t capacity)
{
	assert(window);
	assert(scale > 0.0f);

	Interface interface = calloc(1,
		sizeof(Interface_T));

	if (!interface)
		return NULL;

	interface->window = window;
	interface->scale = scale;
	interface->lastElement = NULL;
	interface->isPressed = false;
#ifndef NDEBUG
	interface->isEnumerating = false;
#endif

	if (capacity == 0)
		capacity = MPGX_DEFAULT_CAPACITY;

	InterfaceElement* elements = malloc(
		sizeof(InterfaceElement) * capacity);

	if (!elements)
	{
		destroyInterface(interface);
		return NULL;
	}

	interface->elements = elements;
	interface->elementCapacity = capacity;
	interface->elementCount = 0;
	return interface;
}

Window getInterfaceWindow(Interface interface)
{
	assert(interface);
	return interface->window;
}
size_t getInterfaceElementCount(Interface interface)
{
	assert(interface);
	return interface->elementCount;
}

cmmt_float_t getInterfaceScale(
	Interface interface)
{
	assert(interface);
	return interface->scale;
}
void setInterfaceScale(
	Interface interface,
	cmmt_float_t scale)
{
	assert(interface);
	assert(scale > 0.0f);
	interface->scale = scale;
}

void enumerateInterface(
	Interface interface,
	OnInterfaceElement onElement,
	void* functionArgument)
{
	assert(interface);
	assert(onElement);

#ifndef NDEBUG
	interface->isEnumerating = true;
#endif

	InterfaceElement* elements = interface->elements;
	size_t elementCount = interface->elementCount;

	for (size_t i = 0; i < elementCount; i++)
		onElement(elements[i], functionArgument);

#ifndef NDEBUG
	interface->isEnumerating = false;
#endif
}
void destroyAllInterfaceElements(
	Interface interface,
	bool destroyTransforms)
{
	assert(interface);
	assert(!interface->isEnumerating);

	InterfaceElement* elements = interface->elements;
	size_t elementCount = interface->elementCount;

	if (elementCount == 0)
		return;

	for (size_t i = 0; i < elementCount; i++)
	{
		InterfaceElement element = elements[i];
		element->onDestroy(element->handle);

		if (destroyTransforms)
			destroyTransform(element->transform);

		free(element);
	}

	interface->elementCount = 0;
}

Camera createInterfaceCamera(
	Interface interface)
{
	assert(interface);

	Vec2I windowSize = getWindowSize(interface->window);
	cmmt_float_t scale = interface->scale;

	Vec2F halfSize = vec2F(
		((cmmt_float_t)windowSize.x / scale) * (cmmt_float_t)0.5,
		((cmmt_float_t)windowSize.y / scale) * (cmmt_float_t)0.5);

	return orthoCamera(
		-halfSize.x,
		halfSize.x,
		-halfSize.y,
		halfSize.y,
		0.0f,
		1.0f);
}

void updateInterface(Interface interface)
{
	assert(interface);

	InterfaceElement* elements = interface->elements;
	size_t elementCount = interface->elementCount;
	Window window = interface->window;

	if (elementCount == 0 || !isWindowFocused(window))
		return;

	cmmt_float_t interfaceScale = interface->scale;
	Vec2I windowSize = getWindowSize(window);
	Vec2F cursor = getWindowCursorPosition(window);

	Vec2F size = vec2F(
		(cmmt_float_t)windowSize.x / interfaceScale,
		(cmmt_float_t)windowSize.y / interfaceScale);
	Vec2F halfSize = mulValVec2F(size, (cmmt_float_t)0.5);

	Vec2F cursorPosition = vec2F(
		(cursor.x / interfaceScale) - halfSize.x,
		(size.y - (cursor.y / interfaceScale)) - halfSize.y);

	InterfaceElement newElement = NULL;
	cmmt_float_t elementDistance = INFINITY;

	for (size_t i = 0; i < elementCount; i++)
	{
		InterfaceElement element = elements[i];
		Transform transform = element->transform;

		if (!element->isEnabled | !isTransformActive(transform))
			continue;

		Transform parent = getTransformParent(transform);

		while (parent)
		{
			if (!isTransformActive(parent))
				goto CONTINUE_1;
			parent = getTransformParent(parent);
		}

		if (element->events.onUpdate)
			element->events.onUpdate(element);

		Vec3F position = getTranslationMat4F(
			getTransformModel(transform));
		Vec3F scale = getTransformScale(transform);

		Box2F bounds = element->bounds;

		bounds.minimum = vec2F(
			bounds.minimum.x * scale.x,
			bounds.minimum.y * scale.y);
		bounds.minimum = vec2F(
			bounds.minimum.x + position.x,
			bounds.minimum.y + position.y);
		bounds.maximum = vec2F(
			bounds.maximum.x * scale.x,
			bounds.maximum.y * scale.y);
		bounds.maximum = vec2F(
			bounds.maximum.x + position.x,
			bounds.maximum.y + position.y);

		if (!isPointInBox2F(bounds, cursorPosition))
			continue;

		if (newElement)
		{
			if (position.z < elementDistance)
			{
				newElement = element;
				elementDistance = position.z;
			}
		}
		else
		{
			newElement = element;
		}

	CONTINUE_1:
		continue;
	}

	bool isLeftButtonPressed = getWindowMouseButton(
		window, LEFT_MOUSE_BUTTON);
	InterfaceElement lastElement = interface->lastElement;

	bool isChanged;

	if (isLeftButtonPressed)
	{
		if (!interface->isPressed)
		{
			isChanged = true;
			interface->isPressed = true;
		}
		else
		{
			isChanged = false;
		}
	}
	else
	{
		if (interface->isPressed)
		{
			isChanged = true;
			interface->isPressed = false;
		}
		else
		{
			isChanged = false;
		}
	}

	if (lastElement)
	{
		if (lastElement != newElement)
		{
			lastElement->isPressed = false;
			interface->lastElement = newElement;

			if (lastElement->events.onExit)
				lastElement->events.onExit(lastElement);
			if (newElement && newElement->events.onEnter)
				newElement->events.onEnter(newElement);
		}
		else
		{
			if (isLeftButtonPressed)
			{
				if (!lastElement->isPressed && isChanged)
				{
					lastElement->isPressed = true;

					if (lastElement->events.onPress)
						lastElement->events.onPress(lastElement);
				}
				else
				{
					if (lastElement->events.onStay)
						lastElement->events.onStay(lastElement);
				}
			}
			else
			{
				// TODO: store pressed element instead of bools
				if (lastElement->isPressed && isChanged)
				{
					lastElement->isPressed = false;

					if (lastElement->events.onRelease)
						lastElement->events.onRelease(lastElement);
				}
				else
				{
					if (lastElement->events.onStay)
						lastElement->events.onStay(lastElement);
				}
			}
		}
	}
	else
	{
		if (newElement)
		{
			interface->lastElement = newElement;

			if (newElement->events.onEnter)
				newElement->events.onEnter(newElement);
		}
	}

	for (size_t i = 0; i < elementCount; i++)
	{
		InterfaceElement element = elements[i];
		Transform transform = element->transform;

		if (!isTransformActive(transform))
			continue;

		Vec2F offset;
		Transform parent = getTransformParent(transform);

		if (parent)
		{
			Vec3F scale = getTransformScale(parent);
			offset.x = scale.x * (cmmt_float_t)0.5;
			offset.y = scale.y * (cmmt_float_t)0.5;
		}
		else
		{
			offset = halfSize;
		}

		while (parent)
		{
			if (!isTransformActive(parent))
				goto CONTINUE_2;
			parent = getTransformParent(parent);
		}

		AlignmentType alignment = element->alignment;
		Vec3F position = element->position;

		switch (alignment)
		{
		default:
			abort();
		case CENTER_ALIGNMENT_TYPE:
			break;
		case LEFT_ALIGNMENT_TYPE:
			position = vec3F(
				position.x - offset.x,
				position.y,
				position.z);
			break;
		case RIGHT_ALIGNMENT_TYPE:
			position = vec3F(
				position.x + offset.x,
				position.y,
				position.z);
			break;
		case BOTTOM_ALIGNMENT_TYPE:
			position = vec3F(
				position.x,
				position.y - offset.y,
				position.z);
			break;
		case TOP_ALIGNMENT_TYPE:
			position = vec3F(
				position.x,
				position.y + offset.y,
				position.z);
			break;
		case LEFT_BOTTOM_ALIGNMENT_TYPE:
			position = vec3F(
				position.x - offset.x,
				position.y - offset.y,
				position.z);
			break;
		case LEFT_TOP_ALIGNMENT_TYPE:
			position = vec3F(
				position.x - offset.x,
				position.y + offset.y,
				position.z);
			break;
		case RIGHT_BOTTOM_ALIGNMENT_TYPE:
			position = vec3F(
				position.x + offset.x,
				position.y - offset.y,
				position.z);
			break;
		case RIGHT_TOP_ALIGNMENT_TYPE:
			position = vec3F(
				position.x + offset.x,
				position.y + offset.y,
				position.z);
			break;
		}

		setTransformPosition(
			transform,
			position);

	CONTINUE_2:
		continue;
	}
}

InterfaceElement createInterfaceElement(
	Interface interface,
	const char* name,
	Transform transform,
	AlignmentType alignment,
	Vec3F position,
	Box2F bounds,
	bool isEnabled,
	OnInterfaceElementDestroy onDestroy,
	const InterfaceElementEvents* events,
	void* handle)
{
	assert(interface);
	assert(transform);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(onDestroy);
	assert(handle);
	assert(!interface->isEnumerating);

	InterfaceElement element = malloc(
		sizeof(InterfaceElement_T));

	if (!element)
		return NULL;

	element->interface = interface;
	element->onDestroy = onDestroy;
	element->events = events ? *events : emptyInterfaceElementEvents;
	element->handle = handle;
	element->transform = transform;
	element->position = position;
	element->bounds = bounds;
	element->alignment = alignment;
	element->isEnabled = isEnabled;
	element->isPressed = false;
#ifndef NDEBUG
	element->name = name;
#endif

	size_t count = interface->elementCount;

	if (count == interface->elementCapacity)
	{
		size_t capacity = interface->elementCapacity * 2;

		InterfaceElement* elements = realloc(
			interface->elements,
			sizeof(InterfaceElement) * capacity);

		if (!elements)
		{
			free(element);
			return NULL;
		}

		interface->elements = elements;
		interface->elementCapacity = capacity;
	}

	interface->elements[count] = element;
	interface->elementCount = count + 1;
	return element;
}
void destroyInterfaceElement(InterfaceElement element)
{
	if (!element)
		return;

	assert(!element->interface->isEnumerating);

	Interface interface = element->interface;
	InterfaceElement* elements = interface->elements;
	size_t elementCount = interface->elementCount;

	for (size_t i = 0; i < elementCount; i++)
	{
		if (elements[i] != element)
			continue;

		for (size_t j = i + 1; j < elementCount; j++)
			elements[j - 1] = elements[j];

		element->onDestroy(element->handle);

		free(element);
		interface->elementCount--;
		return;
	}

	abort();
}

Interface getInterfaceElementInterface(InterfaceElement element)
{
	assert(element);
	return element->interface;
}
const char* getInterfaceElementName(InterfaceElement element)
{
	assert(element);
#ifndef NDEBUG
	return element->name;
#else
	abort();
#endif
}
Transform getInterfaceElementTransform(InterfaceElement element)
{
	assert(element);
	return element->transform;
}
OnInterfaceElementDestroy getInterfaceElementOnDestroy(InterfaceElement element)
{
	assert(element);
	return element->onDestroy;
}
const InterfaceElementEvents* getInterfaceElementEvents(InterfaceElement element)
{
	assert(element);
	return &element->events;
}
void* getInterfaceElementHandle(InterfaceElement element)
{
	assert(element);
	return element->handle;
}

AlignmentType getInterfaceElementAnchor(
	InterfaceElement element)
{
	assert(element);
	return element->alignment;
}
void setInterfaceElementAnchor(
	InterfaceElement element,
	AlignmentType alignment)
{
	assert(element);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	element->alignment = alignment;
}

Vec3F getInterfaceElementPosition(
	InterfaceElement element)
{
	assert(element);
	return element->position;
}
void setInterfaceElementPosition(
	InterfaceElement element,
	Vec3F position)
{
	assert(element);
	element->position = position;
}

Box2F getInterfaceElementBounds(
	InterfaceElement element)
{
	assert(element);
	return element->bounds;
}
void setInterfaceElementBounds(
	InterfaceElement element,
	Box2F bounds)
{
	assert(element);
	element->bounds = bounds;
}

bool isInterfaceElementEnabled(
	InterfaceElement element)
{
	assert(element);
	return element->isEnabled;
}
void setInterfaceElementEnabled(
	InterfaceElement element,
	bool isEnabled)
{
	assert(element);

	if (isEnabled)
	{
		if (!element->isEnabled)
		{
			if (element->events.onEnable)
				element->events.onEnable(element);
			element->isEnabled = true;
		}
	}
	else
	{
		if (element->isEnabled)
		{
			if (element->events.onDisable)
				element->events.onDisable(element);
			element->isEnabled = false;
		}
	}
}
