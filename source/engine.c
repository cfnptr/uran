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
#include "uran/text.h"
#include "uran/defines.h"
#include "uran/shader_data.h"

#include "mpmt/common.h"
#include "mpio/directory.h"

#ifndef NDEBUG
#include "uran/editor.h"
#endif

#include <stdio.h>

#if __linux__ || __APPLE__
#include <sys/utsname.h>
#elif _WIN32
#undef interface
#endif

#define ENGINE_FONT_ATLAS_COUNT 3

struct Engine_T
{
	OnEngineUpdate onUpdate;
	OnEngineRender onRender;
	OnEngineRender onDraw;
	void* argument;
	Logger logger;
	ThreadPool renderingThreadPool;
	ThreadPool backgroundThreadPool;
	Window window;
	PackReader packReader;
	Transformer transformer;
	UserInterface ui;
#ifndef NDEBUG
	Editor editor;
#endif
};

inline static Logger createLoggerInstance(const char* appName)
{
	assert(appName);

	Logger logger;

#if __linux__ || _WIN32
	const char* logDirectoryPath = ".";
#elif __APPLE__
	const char* logDirectoryPath = appName;
#endif

#ifndef NDEBUG
	LogLevel logLevel = ALL_LOG_LEVEL;
	bool logToStdout = true;
#else
	LogLevel logLevel = INFO_LOG_LEVEL;
	bool logToStdout = false;
#endif

	LogyResult logyResult = createLogger(
		logDirectoryPath,
		logLevel,
		logToStdout,
		0.0f,
		true,
		&logger);

	if (logyResult != SUCCESS_LOGY_RESULT)
	{
		fprintf(stderr, "Failed to create logger. (error: %s)\n",
			logyResultToString(logyResult));
		return NULL;
	}

	return logger;
}
inline static void logSystemInfo(
	Logger logger,
	const char* appName,
	Version appVersion)
{
	assert(logger);
	assert(appName);

	logMessage(logger, INFO_LOG_LEVEL,
		"%s [v%hhu.%hhu.%hhu] | "
		"Uran Engine [v" URAN_VERSION_STRING "]",
		appName,
		getVersionMajor(appVersion),
		getVersionMinor(appVersion),
		getVersionPatch(appVersion));

#if __linux__ || __APPLE__
	struct utsname unameData;
	int result = uname(&unameData);

	if (result == 0)
	{
		logMessage(logger, INFO_LOG_LEVEL,
			"OS: %s | %s | %s | %s",
			unameData.sysname, unameData.release,
			unameData.version, unameData.machine);
	}
	else
	{
#if __linux__
		logMessage(logger, INFO_LOG_LEVEL, "OS: Unknown Linux.");
#else
		logMessage(logger, INFO_LOG_LEVEL, "OS: Unknown macOS.");
#endif
	}
#elif _WIN32
	logMessage(logger, INFO_LOG_LEVEL, "OS: Windows.");
#else
#error Unknown operating system
#endif

	logMessage(logger, INFO_LOG_LEVEL,
		"CPU: %s", getCpuName());
	logMessage(logger, INFO_LOG_LEVEL,
		"Logical CPU count: %d", getCpuCount());
	logMessage(logger, INFO_LOG_LEVEL,
		"Total RAM size: %llu", getRamSize());
}
inline static void logGraphicsInfo(
	GraphicsAPI graphicsAPI,
	Logger logger,
	Window window)
{
	assert(graphicsAPI < GRAPHICS_API_COUNT);
	assert(logger);
	assert(window);

	logMessage(logger, INFO_LOG_LEVEL,
		"Graphics API: %s", graphicsApiToString(graphicsAPI));
	logMessage(logger, INFO_LOG_LEVEL,
		"GPU: %s", getWindowGpuName(window));
	logMessage(logger, INFO_LOG_LEVEL,
		"GPU driver: %s", getWindowGpuDriver(window));

	// TODO: log available GPU memory size
}
inline static GraphicsPipeline createPanelPipelineInstance(
	Logger logger,
	Window window,
	PackReader packReader)
{
	assert(logger);
	assert(window);
	assert(packReader);

	const char* vertexShaderPath;
	const char* fragmentShaderPath;
	GraphicsAPI api = getGraphicsAPI();

	if (api == VULKAN_GRAPHICS_API)
	{
		vertexShaderPath = "shaders/vulkan/panel.vert.spv";
		fragmentShaderPath = "shaders/vulkan/panel.frag.spv";
	}
	else if (api == OPENGL_GRAPHICS_API)
	{
		vertexShaderPath = "shaders/opengl/panel.vert";
		fragmentShaderPath = "shaders/opengl/panel.frag";
	}
	else
	{
		abort();
	}

	Framebuffer framebuffer = getWindowFramebuffer(window);

	Shader vertexShader = createShaderFromPack(
		vertexShaderPath,
		VERTEX_SHADER_TYPE,
		packReader, window, logger);

	if (!vertexShader)
		return NULL;

	Shader fragmentShader = createShaderFromPack(
		fragmentShaderPath,
		FRAGMENT_SHADER_TYPE,
		packReader, window, logger);

	if (!fragmentShader)
	{
		destroyShader(vertexShader);
		return NULL;
	}

	GraphicsMesh mesh;

	MpgxResult mpgxResult = createPanelMesh(window, &mesh);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create panel mesh. (error: %s)",
			mpgxResultToString(mpgxResult));
		destroyShader(fragmentShader);
		destroyShader(vertexShader);
		return NULL;
	}

	GraphicsPipeline pipeline;

	mpgxResult = createPanelPipeline(
		framebuffer,
		vertexShader,
		fragmentShader,
		mesh,
		NULL,
		true,
		&pipeline);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create panel pipeline. (error: %s)",
			mpgxResultToString(mpgxResult));
		destroyPanelMesh(mesh);
		destroyShader(fragmentShader);
		destroyShader(vertexShader);
		return NULL;
	}

	return pipeline;
}
inline static void destroyPanelPipelineInstance(
	GraphicsPipeline panelPipeline)
{
	if (!panelPipeline)
		return;

	Shader vertexShader = getGraphicsPipelineShaders(panelPipeline)[0];
	Shader fragmentShader = getGraphicsPipelineShaders(panelPipeline)[1];
	GraphicsMesh mesh = getPanelPipelineMesh(panelPipeline);
	destroyGraphicsPipeline(panelPipeline);
	destroyPanelMesh(mesh);
	destroyShader(fragmentShader);
	destroyShader(vertexShader);
}
inline static GraphicsPipeline createTextPipelineInstance(
	Logger logger,
	Window window,
	PackReader packReader)
{
	assert(logger);
	assert(window);
	assert(packReader);

	const char* vertexShaderPath;
	const char* fragmentShaderPath;
	GraphicsAPI api = getGraphicsAPI();

	if (api == VULKAN_GRAPHICS_API)
	{
		vertexShaderPath = "shaders/vulkan/text.vert.spv";
		fragmentShaderPath = "shaders/vulkan/text.frag.spv";
	}
	else if (api == OPENGL_GRAPHICS_API)
	{
		vertexShaderPath = "shaders/opengl/text.vert";
		fragmentShaderPath = "shaders/opengl/text.frag";
	}
	else
	{
		abort();
	}

	Framebuffer framebuffer = getWindowFramebuffer(window);

	Shader vertexShader = createShaderFromPack(
		vertexShaderPath,
		VERTEX_SHADER_TYPE,
		packReader, window, logger);

	if (!vertexShader)
		return NULL;

	Shader fragmentShader = createShaderFromPack(
		fragmentShaderPath,
		FRAGMENT_SHADER_TYPE,
		packReader, window, logger);

	if (!fragmentShader)
	{
		destroyShader(vertexShader);
		return NULL;
	}

	Sampler sampler;

	MpgxResult mpgxResult = createTextSampler(window, &sampler);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create text sampler. (error: %s)",
			mpgxResultToString(mpgxResult));
		destroyShader(fragmentShader);
		destroyShader(vertexShader);
		return NULL;
	}

	GraphicsPipeline pipeline;

	mpgxResult = createTextPipeline(
		framebuffer,
		vertexShader,
		fragmentShader,
		sampler,
		NULL,
		true,
		1,
		&pipeline);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create text pipeline. (error: %s)",
			mpgxResultToString(mpgxResult));
		destroySampler(sampler);
		destroyShader(fragmentShader);
		destroyShader(vertexShader);
		return NULL;
	}

	return pipeline;
}
inline static void destroyTextPipelineInstance(
	GraphicsPipeline textPipeline)
{
	if (!textPipeline)
		return;

	Shader vertexShader = getGraphicsPipelineShaders(textPipeline)[0];
	Shader fragmentShader = getGraphicsPipelineShaders(textPipeline)[1];
	Sampler sampler = getTextPipelineSampler(textPipeline);
	destroyGraphicsPipeline(textPipeline);
	destroySampler(sampler);
	destroyShader(fragmentShader);
	destroyShader(vertexShader);
}
inline static bool createFontAtlasInstances(
	Logger logger,
	PackReader packReader,
	GraphicsPipeline textPipeline,
	FontAtlas* fontAtlases)
{
	assert(logger);
	assert(packReader);
	assert(textPipeline);
	assert(fontAtlases);

	Font regularFont = createFontFromPack(
		"fonts/dejavu-regular.ttf", 0,
		packReader, logger);

	if (!regularFont)
		return false;

	Font boldFont = createFontFromPack(
		"fonts/dejavu-bold.ttf", 0,
		packReader, logger);

	if (!boldFont)
	{
		destroyFont(regularFont);
		return false;
	}

	Font italicFont = createFontFromPack(
		"fonts/dejavu-italic.ttf", 0,
		packReader, logger);

	if (!italicFont)
	{
		destroyFont(boldFont);
		destroyFont(regularFont);
		return false;
	}

	Font boldItalicFont = createFontFromPack(
		"fonts/dejavu-bold-italic.ttf", 0,
		packReader, logger);

	if (!boldItalicFont)
	{
		destroyFont(italicFont);
		destroyFont(boldFont);
		destroyFont(regularFont);
		return false;
	}

	cmmt_float_t platformScale = getPlatformScale(
		getGraphicsPipelineFramebuffer(textPipeline));

	MpgxResult mpgxResult = createAsciiFontAtlas(
		textPipeline,
		&regularFont,
		&boldFont,
		&italicFont,
		&boldItalicFont,
		1,
		getPlatformFontSize(
			platformScale, 12),
		logger,
		&fontAtlases[0]);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create 12p font atlas. (error: %s)",
			mpgxResultToString(mpgxResult));
		destroyFont(boldItalicFont);
		destroyFont(italicFont);
		destroyFont(boldFont);
		destroyFont(regularFont);
		return false;
	}

	mpgxResult = createAsciiFontAtlas(
		textPipeline,
		&regularFont,
		&boldFont,
		&italicFont,
		&boldItalicFont,
		1,
		getPlatformFontSize(
			platformScale, 14),
		logger,
		&fontAtlases[1]);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create 14p font atlas. (error: %s)",
			mpgxResultToString(mpgxResult));
		destroyFontAtlas(fontAtlases[0]);
		destroyFont(boldItalicFont);
		destroyFont(italicFont);
		destroyFont(boldFont);
		destroyFont(regularFont);
		return false;
	}

	mpgxResult = createAsciiFontAtlas(
		textPipeline,
		&regularFont,
		&boldFont,
		&italicFont,
		&boldItalicFont,
		1,
		getPlatformFontSize(
			platformScale, 16),
		logger,
		&fontAtlases[2]);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create 16p font atlas. (error: %s)",
			mpgxResultToString(mpgxResult));
		destroyFontAtlas(fontAtlases[1]);
		destroyFontAtlas(fontAtlases[0]);
		destroyFont(boldItalicFont);
		destroyFont(italicFont);
		destroyFont(boldFont);
		destroyFont(regularFont);
		return false;
	}

	return true;
}
inline static void destroyFontAtlasInstances(
	FontAtlas* fontAtlases)
{
	assert(fontAtlases);

	Font regularFont = NULL, boldFont = NULL,
		italicFont = NULL, boldItalicFont = NULL;

	if (fontAtlases[0])
	{
		FontAtlas fontAtlas = fontAtlases[0];
		regularFont = getFontAtlasRegularFonts(fontAtlas)[0];
		boldFont = getFontAtlasBoldFonts(fontAtlas)[0];
		italicFont = getFontAtlasItalicFonts(fontAtlas)[0];
		boldItalicFont = getFontAtlasBoldItalicFonts(fontAtlas)[0];
	}

	destroyFontAtlas(fontAtlases[2]);
	destroyFontAtlas(fontAtlases[1]);
	destroyFontAtlas(fontAtlases[0]);
	destroyFont(boldItalicFont);
	destroyFont(italicFont);
	destroyFont(boldFont);
	destroyFont(regularFont);
}
inline static UserInterface createUserInterfaceInstance(
	Logger logger,
	ThreadPool threadPool,
	Window window,
	PackReader packReader)
{
	assert(logger);
	assert(threadPool);
	assert(window);
	assert(packReader);

	GraphicsPipeline panelPipeline = createPanelPipelineInstance(
		logger, window, packReader);

	if (!panelPipeline)
		return NULL;

	GraphicsPipeline textPipeline = createTextPipelineInstance(
		logger, window, packReader);

	if (!textPipeline)
	{
		destroyPanelPipelineInstance(panelPipeline);
		return NULL;
	}

	FontAtlas fontAtlases[ENGINE_FONT_ATLAS_COUNT];

	bool result = createFontAtlasInstances(
		logger, packReader, textPipeline, fontAtlases);

	if (!result)
	{
		destroyTextPipelineInstance(textPipeline);
		destroyPanelPipelineInstance(panelPipeline);
		return NULL;
	}

	UserInterface ui;

	MpgxResult mpgxResult = createUserInterface(
		panelPipeline,
		textPipeline,
		fontAtlases,
		ENGINE_FONT_ATLAS_COUNT,
		1.0f,
		1,
		threadPool,
		&ui);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create user interface. (error: %s)",
			mpgxResultToString(mpgxResult));
		destroyFontAtlasInstances(fontAtlases);
		destroyTextPipelineInstance(textPipeline);
		destroyPanelPipelineInstance(panelPipeline);
		return NULL;
	}

	return ui;
}
inline static void destroyUserInterfaceInstance(UserInterface ui)
{
	if (!ui)
		return;

	GraphicsPipeline panelPipeline = getGraphicsRendererPipeline(
		getUserInterfacePanelRenderer(ui));
	GraphicsPipeline textPipeline = getGraphicsRendererPipeline(
		getUserInterfaceTextRenderer(ui));
	FontAtlas* fontAtlases = getUserInterfaceFontAtlases(ui);
	destroyFontAtlasInstances(fontAtlases);
	destroyUserInterface(ui);
	destroyTextPipelineInstance(textPipeline);
	destroyPanelPipelineInstance(panelPipeline);
}

inline static GraphicsRendererResult onEngineDraw(Engine engine)
{
	assert(engine);
	drawUserInterface(engine->ui);
	return createGraphicsRendererResult();
}
inline static GraphicsRendererResult onEngineRender(Engine engine)
{
	assert(engine);
	engine->onRender(engine->argument);

	Framebuffer framebuffer =
		getWindowFramebuffer(engine->window);
	FramebufferClear clearValues[2];
	clearValues[0].color = zeroLinearColor;
	clearValues[1].depthStencil.depth = 1.0f;
	clearValues[1].depthStencil.stencil = 0;

	beginFramebufferRender(
		framebuffer,
		clearValues,
		2);

	GraphicsRendererResult result = engine->onDraw(engine->argument);
	GraphicsRendererResult tmpResult = onEngineDraw(engine);
	result = addGraphicsRendererResult(result, tmpResult);

	endFramebufferRender(framebuffer);
	return result;
}
static void onEngineUpdate(void* argument)
{
	assert(argument);
	Engine engine = (Engine)argument;
	Window window = engine->window;

	engine->onUpdate(engine->argument);
#ifndef NDEBUG
	updateEditor(engine->editor);
#endif
	updateTransformer(engine->transformer);
	updateUserInterface(engine->ui);
#ifndef NDEBUG
	postUpdateEditor(engine->editor);
#endif

	beginWindowRecord(window);
	GraphicsRendererResult renderResult = onEngineRender(engine);
	endWindowRecord(window);

#ifndef NDEBUG
	setEditorRendererResult(engine->editor, renderResult);
#endif
}

Engine createEngine(
	const char* appName,
	Version appVersion,
	const char* resourcesPath,
	OnEngineUpdate onUpdate,
	OnEngineRender onRender,
	OnEngineRender onDraw,
	void* argument)
{
	assert(appName);
	assert(resourcesPath);
	assert(onUpdate);
	assert(onRender);
	assert(onRender);
	assert(argument);

	Engine engine = calloc(1, sizeof(Engine_T));

	if (!engine)
		return NULL;

	engine->onUpdate = onUpdate;
	engine->onRender = onRender;
	engine->onDraw = onDraw;
	engine->argument = argument;

	Logger logger = createLoggerInstance(appName);

	if (!logger)
	{
		destroyEngine(engine);
		return NULL;
	}

	engine->logger = logger;
	logSystemInfo(logger, appName, appVersion);

	int cpuCount = getCpuCount();

	ThreadPool renderingThreadPool = createThreadPool(
		cpuCount, cpuCount, STACK_TASK_ORDER);

	if (!renderingThreadPool)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create rendering thread pool.");
		destroyEngine(engine);
		return NULL;
	}

	engine->renderingThreadPool = renderingThreadPool;

	ThreadPool backgroundThreadPool = createThreadPool(
		cpuCount, cpuCount * 2, QUEUE_TASK_ORDER);

	if (!backgroundThreadPool)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create background thread pool.");
		destroyEngine(engine);
		return NULL;
	}

	engine->backgroundThreadPool = backgroundThreadPool;

	MpgxResult mpgxResult = initializeGraphics(
		VULKAN_GRAPHICS_API,
		URAN_NAME_STRING,
		URAN_VERSION_MAJOR,
		URAN_VERSION_MINOR,
		URAN_VERSION_PATCH,
		appName,
		getVersionMajor(appVersion),
		getVersionMinor(appVersion),
		getVersionPatch(appVersion));

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, WARN_LOG_LEVEL,
			"Failed to initialize Vulkan graphics subsystem. (error: %s)",
			mpgxResultToString(mpgxResult));

		mpgxResult = initializeGraphics(
			OPENGL_GRAPHICS_API,
			URAN_NAME_STRING,
			URAN_VERSION_MAJOR,
			URAN_VERSION_MINOR,
			URAN_VERSION_PATCH,
			appName,
			getVersionMajor(appVersion),
			getVersionMinor(appVersion),
			getVersionPatch(appVersion));

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			logMessage(logger, ERROR_LOG_LEVEL,
				"Failed to initialize OpenGL graphics subsystem. (error: %s)",
				mpgxResultToString(mpgxResult));
			destroyEngine(engine);
			return NULL;
		}
	}

	bool result = initializeText(logger);

	if (!result)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to initialize text subsystem.");
		destroyEngine(engine);
		return NULL;
	}

	Window window;

	mpgxResult = createWindow(
		onEngineUpdate,
		engine,
		false,
		false,
		false,
		NULL,
		&window);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create window. (error: %s)",
			mpgxResultToString(mpgxResult));
		destroyEngine(engine);
		return NULL;
	}

	engine->window = window;
	logGraphicsInfo(getGraphicsAPI(), logger, window);
	setWindowTitle(window, appName);

	PackReader packReader;

	PackResult packResult = createFilePackReader(
		resourcesPath,
		0,
		true,
		&packReader);

	if (packResult != SUCCESS_PACK_RESULT)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create pack reader. (error: %s)",
			packResultToString(packResult));
		destroyEngine(engine);
		return NULL;
	}

	engine->packReader = packReader;

	Transformer transformer = createTransformer(1, renderingThreadPool);

	if (!transformer)
	{
		destroyEngine(engine);
		return NULL;
	}

	engine->transformer = transformer;

	UserInterface ui = createUserInterfaceInstance(
		logger, renderingThreadPool, window, packReader);

	if (!ui)
	{
		destroyEngine(engine);
		return NULL;
	}

	engine->ui = ui;

#ifndef NDEBUG
	Editor editor = createEditor(logger, window, ui);

	if (!editor)
	{
		logMessage(logger, ERROR_LOG_LEVEL,
			"Failed to create editor.");
		destroyEngine(engine);
		return NULL;
	}

	engine->editor = editor;
#endif

	logMessage(logger, INFO_LOG_LEVEL,
		"Engine initialized.");
	showWindow(window);
	return engine;
}
void destroyEngine(Engine engine)
{
	if (!engine)
		return;

	if (engine->window)
		hideWindow(engine->window);
	if (engine->backgroundThreadPool)
		waitThreadPool(engine->renderingThreadPool);
	if (engine->renderingThreadPool)
		waitThreadPool(engine->backgroundThreadPool);

#ifndef NDEBUG
	destroyEditor(engine->editor);
#endif

	destroyUserInterfaceInstance(engine->ui);
	destroyTransformer(engine->transformer);
	destroyPackReader(engine->packReader);
	destroyWindow(engine->window);
	terminateText(engine->logger);
	terminateGraphics();
	destroyThreadPool(engine->backgroundThreadPool);
	destroyThreadPool(engine->renderingThreadPool);

	if (engine->logger)
	{
		logMessage(engine->logger,
			INFO_LOG_LEVEL, "Engine terminated.");
		destroyLogger(engine->logger);
	}

	free(engine);
}

void joinEngine(Engine engine)
{
	assert(engine);
	joinWindow(engine->window);
}
Logger getEngineLogger(Engine engine)
{
	assert(engine);
	return engine->logger;
}
ThreadPool getEngineRenderingThreadPool(Engine engine)
{
	assert(engine);
	return engine->renderingThreadPool;
}
ThreadPool getEngineBackgroundThreadPool(Engine engine)
{
	assert(engine);
	return engine->backgroundThreadPool;
}
Window getEngineWindow(Engine engine)
{
	assert(engine);
	return engine->window;
}
PackReader getEnginePackReader(Engine engine)
{
	assert(engine);
	return engine->packReader;
}
Transformer getEngineTransformer(Engine engine)
{
	assert(engine);
	return engine->transformer;
}
UserInterface getEngineUserInterface(Engine engine)
{
	assert(engine);
	return engine->ui;
}

void destroyEnginePackReader(Engine engine)
{
	assert(engine);
	destroyPackReader(engine->packReader);
	engine->packReader = NULL;
}

//  make windows-pack script
//  share WebP image reader cache
