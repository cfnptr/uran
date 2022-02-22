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

#include "program.h"
#include "mpmt/common.h"

#include <stdio.h>

#if _WIN32
#ifdef NDEBUG
#include <windows.h>
#define MAIN_FUNCTION int APIENTRY WinMain(                          \
	HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
#else
#define MAIN_FUNCTION int main(int argc, char *argv[])
#endif
#else
#define MAIN_FUNCTION int main(int argc, char *argv[])
#endif

#define LOG_FILE_PATH "log.txt"

static void onUpdate(void* argument)
{
	assert(argument);

	Program program = *(Program*)argument;
	updateProgram(program);

	Window window = getProgramWindow(program);
	beginWindowRecord(window);
	renderProgram(program);
	endWindowRecord(window);
}

MAIN_FUNCTION
{
	remove(LOG_FILE_PATH);

#ifndef NDEBUG
	LogLevel logLevel = ALL_LOG_LEVEL;
	bool logToStdout = true;
#else
	LogLevel logLevel = INFO_LOG_LEVEL;
	bool logToStdout = false;
#endif

	Logger logger;

	LogyResult logyResult = createLogger(
		LOG_FILE_PATH,
		logLevel,
		logToStdout,
		&logger);

	if (logyResult != SUCCESS_LOGY_RESULT)
	{
		printf("Failed to create logger. (error: %s)\n",
			logyResultToString(logyResult));
		return EXIT_FAILURE;
	}

	logMessage(logger, INFO_LOG_LEVEL,
#if __linux__
		"OS: Linux.");
#elif __APPLE__
		"OS: macOS.");
#elif _WIN32
		"OS: Windows.");
#else
#error Unknown operating system
#endif

	logMessage(logger, INFO_LOG_LEVEL,
		"CPU: %s.", getCpuName());

	int cpuCount = getCpuCount();

	logMessage(logger, INFO_LOG_LEVEL,
		"CPU count: %d.", cpuCount);

	ThreadPool threadPool = createThreadPool(
		cpuCount, cpuCount);

	if (!threadPool)
	{
		logMessage(logger, FATAL_LOG_LEVEL,
			"Failed to create thread pool.");
		destroyLogger(logger);
		return EXIT_FAILURE;
	}

	Program program;
	Editor editor;

#ifndef NDEBUG
	editor = createEditor(
		logger,
		threadPool,
		onUpdate,
		&program);

	if (!editor)
	{
		logMessage(logger, FATAL_LOG_LEVEL,
			"Failed to create editor.");
		destroyThreadPool(threadPool);
		destroyLogger(logger);
		return EXIT_FAILURE;
	}
#else
	editor = NULL;
#endif

	program = createProgram(
		logger,
		threadPool,
		editor);

	if (!program)
	{
		logMessage(logger, FATAL_LOG_LEVEL,
			"Failed to create program.");
		destroyEditor(editor);
		destroyThreadPool(threadPool);
		destroyLogger(logger);
		return EXIT_FAILURE;
	}

	Window window = getProgramWindow(program);

	logMessage(logger, INFO_LOG_LEVEL,
		"Graphics API: %s.",
		graphicsApiToString(getGraphicsAPI()));
	logMessage(logger, INFO_LOG_LEVEL,
		"GPU: %s.",
		getWindowGpuName(window));

	showWindow(window);
	joinWindow(window);

	destroyProgram(program);
#ifndef NDEBUG
	destroyEditor(editor);
#endif
	destroyThreadPool(threadPool);
	destroyLogger(logger);
	return EXIT_SUCCESS;
}
