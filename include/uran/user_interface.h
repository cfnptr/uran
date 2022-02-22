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
#include "uran/interface.h"
#include "uran/renderers/text_renderer.h"
#include "uran/renderers/sprite_renderer.h"

#include "pack/reader.h"

/*
 * User interface structure.
 */
typedef struct UserInterface_T UserInterface_T;
/*
 * User interface instance.
 */
typedef UserInterface_T* UserInterface;

/*
 * User interface panel instance.
 */
typedef InterfaceElement UiPanel;
/*
 * User interface window instance.
 */
typedef InterfaceElement UiWindow;
/*
 * User interface button instance.
 */
typedef InterfaceElement UiButton;

/*
 * Create a new user interface instance.
 * Returns operation MPGX result.
 *
 * transformer - transformer instance.
 * spritePipeline - sprite pipeline instance.
 * textPipeline - text pipeline instance.
 * capacity - initial element array capacity or 0.
 * threadPool - thread pool instance or NULL.
 * ui - pointer to the user interface instance.
 */
MpgxResult createUserInterface(
	Transformer transformer,
	GraphicsPipeline spritePipeline,
	GraphicsPipeline textPipeline,
	size_t capacity,
	ThreadPool threadPool,
	UserInterface* ui);
/*
 * Destroys user interface instance.
 * ui - user interface instance or NULL.
 */
void destroyUserInterface(UserInterface ui);

/*
 * Returns user interface transformer.
 * ui - user interface instance.
 */
Transformer getUserInterfaceTransformer(UserInterface ui);
/*
 * Returns user interface instance.
 * ui - user interface instance.
 */
Interface getUserInterface(UserInterface ui);
/*
 * Returns user interface sprite pipeline.
 * ui - user interface instance.
 */
GraphicsPipeline getUserInterfaceSpritePipeline(UserInterface ui);
/*
 * Returns user interface text pipeline.
 * ui - user interface instance.
 */
GraphicsPipeline getUserInterfaceTextPipeline(UserInterface ui);

/*
 * Processes user interface events.
 * ui - user interface instance.
 */
void updateUserInterface(UserInterface ui);
/*
 * Draw user interface elements.
 * ui - user interface instance.
 */
GraphicsRendererResult drawUserInterface(UserInterface ui);

UiPanel createUiPanel(
	UserInterface ui,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	LinearColor color,
	Transform parent,
	bool isActive);

UiWindow createUiWindow(
	UserInterface ui,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	float barHeight,
	LinearColor barColor,
	LinearColor panelColor,
	Transform parent,
	bool isActive);

UiButton createUiButton(
	UserInterface ui,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	LinearColor disabledColor,
	LinearColor enabledColor,
	LinearColor hoveredColor,
	LinearColor pressedColor,
	Transform parent,
	bool isEnabled,
	bool isActive);


