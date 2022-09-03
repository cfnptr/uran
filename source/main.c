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

#include "uran/entry.h"
#include "uran/engine.h"
#include "uran/defines.h"

#include <stdio.h>

typedef struct Program
{
	Engine engine;
} Program;

static void onUpdate(void* argument)
{

}
static GraphicsRendererResult onRender(void* argument)
{
	return createGraphicsRendererResult();
}
static GraphicsRendererResult onDraw(void* argument)
{
	return createGraphicsRendererResult();
}

URAN_MAIN_FUNCTION
{
	Program program;

	Engine engine = createEngine(
		URAN_NAME_STRING,
		createVersion(
			URAN_VERSION_MAJOR,
			URAN_VERSION_MINOR,
			URAN_VERSION_PATCH),
		URAN_RESOURCES_FILE_PATH,
		onUpdate, onRender, onDraw,
		&program);
	
	if (!engine)
	{
		fprintf(stderr, "Failed to start engine.");
		return EXIT_FAILURE;
	}

	program.engine = engine;

	joinEngine(engine);
	destroyEngine(engine);
	return EXIT_SUCCESS;
}
