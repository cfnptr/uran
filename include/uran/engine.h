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
#include "uran/version.h"
#include "uran/user_interface.h"

#include "mpgx/window.h"
#include "pack/reader.h"
#include "logy/logger.h"
#include "mpmt/thread_pool.h"

/*
 * Engine structure.
 */
typedef struct Engine_T Engine_T;
/*
 * Engine instance.
 */
typedef Engine_T* Engine;

/*
 * Engine update function.
 * argument - function argument or NULL.
 */
typedef void(*OnEngineUpdate)(void* argument);
/*
 * Engine render function.
 * argument - function argument or NULL.
 */
typedef GraphicsRendererResult(*OnEngineRender)(void* argument);

/*
 * Create a new engine instance.
 * Returns engine instance on success, otherwise NULL.
 *
 * appName - application or game name string.
 * appVersion - application or game version.
 * graphicsAPI - graphics backend API.
 * resourcesPath - resources file path string.
 * onUpdate - on engine update function.
 * onRender - on engine render function.
 * onDraw - on engine draw function.
 * handle - engine functions argument.
 */
Engine createEngine(
	const char* appName,
	Version appVersion,
	const char* resourcesPath,
	OnEngineUpdate onUpdate,
	OnEngineRender onRender,
	OnEngineRender onDraw,
	void* argument);
/*
 * Destroys engine instance.
 * engine - engine instance or NULL.
 */
void destroyEngine(Engine engine);

/*
 * Joins engine update loop.
 * engine - engine instance.
 */
void joinEngine(Engine engine);
/*
 * Returns engine logger.
 * engine - engine instance.
 */
Logger getEngineLogger(Engine engine);
/*
 * Returns engine rendering thread pool.
 * engine - engine instance.
 */
ThreadPool getEngineRenderingThreadPool(Engine engine);
/*
 * Returns engine background thread pool.
 * engine - engine instance.
 */
ThreadPool getEngineBackgroundThreadPool(Engine engine);
/*
 * Returns engine window.
 * engine - engine instance.
 */
Window getEngineWindow(Engine engine);
/*
 * Returns engine pack reader.
 * engine - engine instance.
 */
PackReader getEnginePackReader(Engine engine);
/*
 * Returns engine transformer.
 * engine - engine instance.
 */
Transformer getEngineTransformer(Engine engine);
/*
 * Returns engine user interface.
 * engine - engine instance.
 */
UserInterface getEngineUserInterface(Engine engine);

/*
 * Destroys engine pack reader.
 * engine - engine instance.
 */
void destroyEnginePackReader(Engine engine);
