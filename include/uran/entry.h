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
#include "mpgx/window.h"
#include "logy/logger.h"
#include "mpmt/common.h"

#if _WIN32
#ifdef NDEBUG
#include <windows.h>
#define URAN_MAIN_FUNCTION int APIENTRY WinMain(                     \
	HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
#else
#define URAN_MAIN_FUNCTION int main(int argc, char *argv[])
#endif
#else
#define URAN_MAIN_FUNCTION int main(int argc, char *argv[])
#endif

#if __linux__ || __APPLE__
#include <sys/utsname.h>
#endif

inline static void logSystemInfo(Logger logger)
{
	assert(logger);

#if __linux__ || __APPLE__
	struct utsname unameData;
	int result = uname(&unameData);

	if (result == 0)
	{
		logMessage(logger, INFO_LOG_LEVEL, "OS: %s %s %s %s.",
			unameData.sysname, unameData.release,
			unameData.version, unameData.machine);
	}
	else
	{
#if __linux__
		logMessage(logger, INFO_LOG_LEVEL, "OS: Linux.");
#else
		logMessage(logger, INFO_LOG_LEVEL, "OS: macOS.");
#endif
	}
#elif _WIN32
	// TODO: use RtlGetVersion
	logMessage(logger, INFO_LOG_LEVEL, "OS: Windows.");
#else
#error Unknown operating system
#endif

	int cpuCount = getCpuCount();

	logMessage(logger, INFO_LOG_LEVEL,
		"CPU: %s.", getCpuName());
	logMessage(logger, INFO_LOG_LEVEL,
		"CPU count: %d.", cpuCount);
}
inline static void logWindowInfo(Logger logger, Window window)
{
	assert(logger);

	logMessage(logger, INFO_LOG_LEVEL,
		"GPU: %s.", getWindowGpuName(window));
	logMessage(logger, INFO_LOG_LEVEL,
		"GPU driver: %s.", getWindowGpuDriver(window));
}
