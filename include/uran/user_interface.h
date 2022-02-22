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

MpgxResult createUserInterface(
	Transformer transformer,
	GraphicsPipeline spritePipeline,
	GraphicsPipeline textPipeline,
	size_t capacity,
	ThreadPool threadPool,
	UserInterface* ui);
void destroyUserInterface(UserInterface ui);

Transformer getUserInterfaceTransformer(UserInterface ui);
Interface getUserInterface(UserInterface ui);
GraphicsPipeline getUserInterfaceSpritePipeline(UserInterface ui);
GraphicsPipeline getUserInterfaceTextPipeline(UserInterface ui);

void preUpdateUserInterface(UserInterface ui);
void updateUserInterface(UserInterface ui);
GraphicsRendererResult drawUserInterface(UserInterface ui);

UiPanel createUiPanel(
	UserInterface ui,
	AlignmentType alignment,
	Vec3F position,
	Vec2F scale,
	LinearColor color,
	Transform parent,
	bool isActive);




