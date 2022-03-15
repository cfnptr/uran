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

// =================================================================
// Modify this file to create your custom game or application logic.
// =================================================================

#pragma once
#include "uran/editor.h"

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
 * Updates program.
 * argument - function argument.
 */
static void onProgramUpdate(void* argument)
{
	assert(argument);
	Program program = (Program)argument;
	Editor editor = program->editor;
	Window window = getEditorWindow(editor);
	updateEditor(editor);
	beginWindowRecord(window);
	renderEditor(editor);
	endWindowRecord(window);
}

/*
 * Destroys program instance.
 * program - program instance or NULL.
 */
inline static void destroyProgram(Program program)
{
	if (!program)
		return;

	destroyEditor(program->editor);
	free(program);
}
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
	ThreadPool threadPool)
{
	assert(logger);
	assert(threadPool);

	logMessage(logger, INFO_LOG_LEVEL,
		"Uran - Editor (v" URAN_VERSION_STRING ")");

	Program program = calloc(
		1, sizeof(Program_T));

	if (!program)
		return NULL;

	program->logger = logger;
	program->threadPool = threadPool;

	Editor editor = createEditor(
		logger,
		threadPool,
		onProgramUpdate,
		program);

	if (!editor)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create editor.");
		destroyProgram(program);
		return NULL;
	}

	program->editor = editor;
	return program;
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
