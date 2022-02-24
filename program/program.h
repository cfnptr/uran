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

// ==============================================================
// Modify this file to add your custom game or application logic.
// ==============================================================

#pragma once
#include "uran/editor.h"

/*
 * Create editor instance and pass to the createProgram function.
 */
#define CREATE_EDITOR 1

/*
 * Program structure.
 */
typedef struct Program_T
{
	Logger logger;
	ThreadPool threadPool;
	Editor editor;
} Program_T;
/*
 * Program instance.
 */
typedef Program_T* Program;

/*
 * Create a new program instance.
 * Return program instance on success, otherwise NULL.
 *
 * logger - logger instance.
 * threadPool - thread pool instance.
 * editor - editor instance.
 */
inline static Program createProgram(
	Logger logger,
	ThreadPool threadPool,
	Editor editor)
{
	assert(logger);
	assert(threadPool);
	assert(editor);

	Program program = calloc(1, sizeof(Program_T));

	if (!program)
		return NULL;

	program->logger = logger;
	program->threadPool = threadPool;
	program->editor = editor;
	return program;
}
/*
 * Destroys program instance.
 * program - program instance or NULL.
 */
inline static void destroyProgram(Program program)
{
	if (!program)
		return;

	free(program);
}

/*
 * Updates program.
 * program - program instance.
 */
inline static void updateProgram(Program program)
{
	assert(program);
	updateEditor(program->editor);
}
/*
 * Renders program.
 * program - program instance.
 */
inline static void renderProgram(Program program)
{
	assert(program);
	renderEditor(program->editor);
}

/*
 * Returns program window.
 * program - program instance.
 */
inline static Window getProgramWindow(Program program)
{
	assert(program);
	return getEditorWindow(program->editor);
}
