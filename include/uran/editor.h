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
#include "uran/user_interface.h"

#include "logy/logger.h"
#include "mpgx/window.h"

/*
 * Editor structure.
 */
typedef struct Editor_T Editor_T;
/*
 * Editor instance.
 */
typedef Editor_T* Editor;

/*
 * Create a new editor instance.
 * Returns editor instance on success, otherwise NULL.
 *
 * logger - logger instance.
 * window - window instance.
 * ui - user interface instance.
 */
Editor createEditor(
	Logger logger,
	Window window,
	UserInterface ui);
/*
 * Destroys editor instance.
 * editor - pointer to the editor instance or NULL.
 */
void destroyEditor(Editor editor);

/*
 * Sets editor stats renderer data.
 *
 * editor - editor instance.
 * result - graphics renderer result.
 */
void setEditorRendererResult(
	Editor editor,
	GraphicsRendererResult result);

/*
 * Update editor.
 * editor - editor instance.
 */
void updateEditor(Editor editor);
/*
 * Update editor after all another code.
 * editor - editor instance.
 */
void postUpdateEditor(Editor editor);
