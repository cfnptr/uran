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

#include "uran/engine.h"
#include "uran/entry.h"
#include "uran/defines.h"

#include <stdio.h>

#define APP_NAME_STRING URAN_NAME_STRING " Editor"
#define APP_VERSION createVersion( \
	URAN_VERSION_MAJOR, URAN_VERSION_MINOR, URAN_VERSION_PATCH)
#define APP_RESOURCES_PATH "resources.pack"

typedef struct App
{
	Engine engine;
} App;

static void onUpdate(void* argument)
{
	App* app = (App*)argument;
}
static GraphicsRendererResult onRender(void* argument)
{
	App* app = (App*)argument;
	return createGraphicsRendererResult();
}
static GraphicsRendererResult onDraw(void* argument)
{
	App* app = (App*)argument;
	return createGraphicsRendererResult();
}

URAN_MAIN_FUNCTION
{
	App app;

	Engine engine = createEngine(
		APP_NAME_STRING,
		APP_VERSION,
		APP_RESOURCES_PATH,
		onUpdate,
		onRender,
		onDraw,
		&app);

	if (!engine)
	{
		fprintf(stderr, "Failed to create engine.");
		return EXIT_FAILURE;
	}

	app.engine = engine;
	destroyEnginePackReader(engine);
	joinEngine(engine);

	destroyEngine(engine);
	return EXIT_SUCCESS;
}
