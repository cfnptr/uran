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

#include "uran/pipelines/panel_pipeline.h"
#include "uran/primitives/square_primitive.h"

#include "mpgx/_source/window.h"
#include "mpgx/_source/graphics_mesh.h"
#include "mpgx/_source/graphics_pipeline.h"

#include <string.h>

typedef struct VertexPushConstants
{
	mat4 mvp;
} VertexPushConstants;
typedef struct FragmentPushConstants
{
	vec4 color;
} FragmentPushConstants;
typedef struct BaseHandle
{
	GraphicsMesh mesh;
	VertexPushConstants vpc;
	FragmentPushConstants fpc;
} BaseHandle;
#if MPGX_SUPPORT_VULKAN
typedef struct VkHandle
{
	GraphicsMesh mesh;
	VertexPushConstants vpc;
	FragmentPushConstants fpc;
} VkHandle;
#endif
#if MPGX_SUPPORT_OPENGL
typedef struct GlHandle
{
	GraphicsMesh mesh;
	VertexPushConstants vpc;
	FragmentPushConstants fpc;
	GLint mvpLocation;
	GLint colorLocation;
} GlHandle;
#endif
typedef union Handle_T
{
	BaseHandle base;
#if MPGX_SUPPORT_VULKAN
	VkHandle vk;
#endif
#if MPGX_SUPPORT_OPENGL
	GlHandle gl;
#endif
} Handle_T;

typedef Handle_T* Handle;

#if MPGX_SUPPORT_VULKAN
static const VkVertexInputBindingDescription vertexInputBindingDescriptions[1] = {
	{
		0,
		sizeof(Vec2F),
		VK_VERTEX_INPUT_RATE_VERTEX,
	},
};
static const VkVertexInputAttributeDescription vertexInputAttributeDescriptions[1] = {
	{
		0,
		0,
		VK_FORMAT_R32G32_SFLOAT,
		0,
	},
};
static const VkPushConstantRange pushConstantRanges[2] = {
	{
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		sizeof(VertexPushConstants),
	},
	{
		VK_SHADER_STAGE_FRAGMENT_BIT,
		sizeof(VertexPushConstants),
		sizeof(FragmentPushConstants),
	},
};

static void onVkBind(GraphicsPipeline graphicsPipeline)
{
	assert(graphicsPipeline);
	Handle handle = graphicsPipeline->vk.handle;
	GraphicsMesh mesh = handle->vk.mesh;

	assert(mesh->vk.vertexBuffer);
	assert(mesh->vk.indexBuffer);

	VkWindow vkWindow = getVkWindow(graphicsPipeline->vk.window);
	VkCommandBuffer commandBuffer = vkWindow->currenCommandBuffer;

	const VkDeviceSize offset = 0;

	vkCmdBindVertexBuffers(
		commandBuffer,
		0,
		1,
		&mesh->vk.vertexBuffer->vk.handle,
		&offset);
	vkCmdBindIndexBuffer(
		commandBuffer,
		mesh->vk.indexBuffer->vk.handle,
		0,
		mesh->vk.vkIndexType);
}
static void onVkUniformsSet(GraphicsPipeline graphicsPipeline)
{
	assert(graphicsPipeline);
	Handle handle = graphicsPipeline->vk.handle;
	VkWindow vkWindow = getVkWindow(graphicsPipeline->vk.window);
	VkCommandBuffer commandBuffer = vkWindow->currenCommandBuffer;
	VkPipelineLayout layout = graphicsPipeline->vk.layout;

	vkCmdPushConstants(
		commandBuffer,
		layout,
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		sizeof(VertexPushConstants),
		&handle->vk.vpc);
	vkCmdPushConstants(
		commandBuffer,
		layout,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		sizeof(VertexPushConstants),
		sizeof(FragmentPushConstants),
		&handle->vk.fpc);
}
static void onVkResize(
	GraphicsPipeline graphicsPipeline,
	Vec2I newSize,
	void* vkCreateData)
{
	assert(graphicsPipeline);
	assert(newSize.x > 0);
	assert(newSize.y > 0);
	assert(vkCreateData);

	Vec4I size = vec4I(0, 0,
		newSize.x, newSize.y);

	if (graphicsPipeline->vk.state.viewport.z +
		graphicsPipeline->vk.state.viewport.w)
	{
		graphicsPipeline->vk.state.viewport = size;
	}
	if (graphicsPipeline->vk.state.scissor.z +
		graphicsPipeline->vk.state.scissor.w)
	{
		graphicsPipeline->vk.state.scissor = size;
	}

	VkGraphicsPipelineCreateData createData = {
		1,
		vertexInputBindingDescriptions,
		1,
		vertexInputAttributeDescriptions,
		0,
		NULL,
		2,
		pushConstantRanges,
	};

	*(VkGraphicsPipelineCreateData*)vkCreateData = createData;
}
static void onVkDestroy(
	Window window,
	void* handle)
{
	assert(window);
	free((Handle)handle);
}
inline static MpgxResult createVkPipeline(
	Framebuffer framebuffer,
	Window window,
	const char* name,
	const GraphicsPipelineState* state,
	Handle handle,
	Shader* shaders,
	uint8_t shaderCount,
	GraphicsPipeline* graphicsPipeline)
{
	assert(framebuffer);
	assert(window);
	assert(state);
	assert(handle);
	assert(shaders);
	assert(shaderCount > 0);
	assert(graphicsPipeline);

	VkGraphicsPipelineCreateData createData = {
		1,
		vertexInputBindingDescriptions,
		1,
		vertexInputAttributeDescriptions,
		0,
		NULL,
		2,
		pushConstantRanges,
	};

	MpgxResult mpgxResult = createGraphicsPipeline(
		framebuffer,
		name,
		state,
		onVkBind,
		onVkUniformsSet,
		onVkResize,
		onVkDestroy,
		handle,
		&createData,
		shaders,
		shaderCount,
		graphicsPipeline);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		onVkDestroy(window, handle);
		return mpgxResult;
	}

	return SUCCESS_MPGX_RESULT;
}
#endif

#if MPGX_SUPPORT_OPENGL
static void onGlBind(GraphicsPipeline graphicsPipeline)
{
	assert(graphicsPipeline);
	Handle handle = graphicsPipeline->gl.handle;
	GraphicsMesh mesh = handle->gl.mesh;

	assert(mesh->gl.vertexBuffer);
	assert(mesh->gl.indexBuffer);

	glBindVertexArray(mesh->gl.handle);
	glBindBuffer(
		GL_ARRAY_BUFFER,
		mesh->gl.vertexBuffer->gl.handle);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		mesh->gl.indexBuffer->gl.handle);
	assertOpenGL();
}
static void onGlUniformsSet(GraphicsPipeline graphicsPipeline)
{
	assert(graphicsPipeline);
	Handle handle = graphicsPipeline->gl.handle;

	glUniformMatrix4fv(
		handle->gl.mvpLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&handle->gl.vpc.mvp);
	glUniform4fv(
		handle->gl.colorLocation,
		1,
		(const GLfloat*)&handle->gl.fpc.color);

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(
		0,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec2F),
		0);
	assertOpenGL();
}
static void onGlResize(
	GraphicsPipeline graphicsPipeline,
	Vec2I newSize,
	void* vkCreateData)
{
	assert(graphicsPipeline);
	assert(newSize.x > 0);
	assert(newSize.y > 0);
	assert(!vkCreateData);

	Vec4I size = vec4I(0, 0,
		newSize.x, newSize.y);

	if (graphicsPipeline->gl.state.viewport.z +
		graphicsPipeline->gl.state.viewport.w)
	{
		graphicsPipeline->gl.state.viewport = size;
	}
	if (graphicsPipeline->gl.state.scissor.z +
		graphicsPipeline->gl.state.scissor.w)
	{
		graphicsPipeline->gl.state.scissor = size;
	}
}
static void onGlDestroy(
	Window window,
	void* handle)
{
	assert(window);
	free((Handle)handle);
}
inline static MpgxResult createGlPipeline(
	Framebuffer framebuffer,
	Window window,
	const char* name,
	const GraphicsPipelineState* state,
	Handle handle,
	Shader* shaders,
	uint8_t shaderCount,
	GraphicsPipeline* graphicsPipeline)
{
	assert(framebuffer);
	assert(window);
	assert(state);
	assert(handle);
	assert(shaders);
	assert(shaderCount > 0);
	assert(graphicsPipeline);

	GraphicsPipeline graphicsPipelineInstance;

	MpgxResult mpgxResult = createGraphicsPipeline(
		framebuffer,
		name,
		state,
		onGlBind,
		onGlUniformsSet,
		onGlResize,
		onGlDestroy,
		handle,
		NULL,
		shaders,
		shaderCount,
		&graphicsPipelineInstance);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		onGlDestroy(window, handle);
		return mpgxResult;
	}

	GLuint glHandle = graphicsPipelineInstance->gl.glHandle;

	GLint mvpLocation, colorLocation;

	bool result = getGlUniformLocation(
		glHandle,
		"u_MVP",
		&mvpLocation);
	result &= getGlUniformLocation(
		glHandle,
		"u_Color",
		&colorLocation);

	if (!result)
	{
		destroyGraphicsPipeline(graphicsPipelineInstance);
		return BAD_SHADER_CODE_MPGX_RESULT;
	}

	assertOpenGL();

	handle->gl.mvpLocation = mvpLocation;
	handle->gl.colorLocation = colorLocation;

	*graphicsPipeline = graphicsPipelineInstance;
	return SUCCESS_MPGX_RESULT;
}
#endif

MpgxResult createPanelPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	GraphicsMesh mesh,
	const GraphicsPipelineState* state,
	bool useScissors,
	GraphicsPipeline* panelPipeline)
{
	assert(framebuffer);
	assert(vertexShader);
	assert(fragmentShader);
	assert(mesh);
	assert(panelPipeline);
	assert(vertexShader->base.type == VERTEX_SHADER_TYPE);
	assert(fragmentShader->base.type == FRAGMENT_SHADER_TYPE);
	assert(vertexShader->base.window == framebuffer->base.window);
	assert(fragmentShader->base.window == framebuffer->base.window);
	assert(mesh->base.vertexBuffer);
	assert(mesh->base.indexBuffer);

	Handle handle = calloc(1, sizeof(Handle_T));

	if (!handle)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	vec4 color = {
		1.0f, 1.0f, 1.0f, 1.0f,
	};

	handle->base.mesh = mesh;
	handle->base.fpc.color = color;

#ifndef NDEBUG
	const char* name = PANEL_PIPELINE_NAME;
#else
	const char* name = NULL;
#endif

	Vec2I framebufferSize =
		framebuffer->base.size;
	Vec4I size = vec4I(0, 0,
		framebufferSize.x,
		framebufferSize.y);

	GraphicsPipelineState defaultState = {
		TRIANGLE_LIST_DRAW_MODE,
		FILL_POLYGON_MODE,
		BACK_CULL_MODE,
		LESS_COMPARE_OPERATOR,
		ALL_COLOR_COMPONENT,
		SOURCE_ALPHA_BLEND_FACTOR,
		ONE_MINUS_SOURCE_ALPHA_BLEND_FACTOR,
		ONE_BLEND_FACTOR,
		ZERO_BLEND_FACTOR,
		ADD_BLEND_OPERATOR,
		ADD_BLEND_OPERATOR,
		true,
		true,
		true,
		true,
		false,
		false,
		true,
		false,
		false,
		DEFAULT_LINE_WIDTH,
		size,
		useScissors ? zeroVec4I : size,
		defaultDepthRange,
		defaultDepthBias,
		defaultBlendColor,
	};

	Shader shaders[2] = {
		vertexShader,
		fragmentShader,
	};

	Window window = framebuffer->base.window;
	GraphicsAPI api = getGraphicsAPI();

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		return createVkPipeline(
			framebuffer,
			window,
			name,
			state ? state : &defaultState,
			handle,
			shaders,
			2,
			panelPipeline);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		return createGlPipeline(
			framebuffer,
			window,
			name,
			state ? state : &defaultState,
			handle,
			shaders,
			2,
			panelPipeline);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}
}

MpgxResult createPanelMesh(
	Window window,
	GraphicsMesh* panelMesh)
{
	assert(window);
	assert(panelMesh);

	Buffer vertexBuffer;

	MpgxResult mpgxResult = createBuffer(window,
		VERTEX_BUFFER_TYPE,
		GPU_ONLY_BUFFER_USAGE,
		oneSquareVertices2D,
		sizeof(oneSquareVertices2D),
		&vertexBuffer);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	Buffer indexBuffer;

	mpgxResult = createBuffer(window,
		INDEX_BUFFER_TYPE,
		GPU_ONLY_BUFFER_USAGE,
		triangleSquareIndices,
		sizeof(triangleSquareIndices),
		&indexBuffer);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyBuffer(vertexBuffer);
		return mpgxResult;
	}

	GraphicsMesh mesh;

	mpgxResult = createGraphicsMesh(window,
		UINT16_INDEX_TYPE,
		sizeof(triangleSquareIndices) / sizeof(uint16_t),
		0,
		vertexBuffer,
		indexBuffer,
		&mesh);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyBuffer(vertexBuffer);
		destroyBuffer(indexBuffer);
		return mpgxResult;
	}

	*panelMesh = mesh;
	return SUCCESS_MPGX_RESULT;
}
void destroyPanelMesh(GraphicsMesh panelMesh)
{
	if (!panelMesh)
		return;

	Buffer vertexBuffer = getGraphicsMeshVertexBuffer(panelMesh);
	Buffer indexBuffer = getGraphicsMeshIndexBuffer(panelMesh);
	destroyGraphicsMesh(panelMesh);
	destroyBuffer(indexBuffer);
	destroyBuffer(vertexBuffer);
}

GraphicsMesh getPanelPipelineMesh(
	GraphicsPipeline panelPipeline)
{
	assert(panelPipeline);
	assert(strcmp(panelPipeline->base.name,
		PANEL_PIPELINE_NAME) == 0);
	Handle handle = panelPipeline->base.handle;
	return handle->base.mesh;
}
void setPanelPipelineMesh(
	GraphicsPipeline panelPipeline,
	GraphicsMesh mesh)
{
	assert(panelPipeline);
	assert(mesh);
	assert(mesh->base.vertexBuffer);
	assert(mesh->base.indexBuffer);
	assert(strcmp(panelPipeline->base.name,
		PANEL_PIPELINE_NAME) == 0);
	Handle handle = panelPipeline->base.handle;
	handle->base.mesh = mesh;
}

const mat4* getPanelPipelineMvp(
	GraphicsPipeline panelPipeline)
{
	assert(panelPipeline);
	assert(strcmp(panelPipeline->base.name,
		PANEL_PIPELINE_NAME) == 0);
	Handle handle = panelPipeline->base.handle;
	return &handle->base.vpc.mvp;
}
void setPanelPipelineMvp(
	GraphicsPipeline panelPipeline,
	const Mat4F* mvp)
{
	assert(panelPipeline);
	assert(mvp);
	assert(strcmp(panelPipeline->base.name,
		PANEL_PIPELINE_NAME) == 0);
	Handle handle = panelPipeline->base.handle;
	handle->base.vpc.mvp = cmmtToMat4(*mvp);
}

vec4 getPanelPipelineColor(
	GraphicsPipeline panelPipeline)
{
	assert(panelPipeline);
	assert(strcmp(panelPipeline->base.name,
		PANEL_PIPELINE_NAME) == 0);
	Handle handle = panelPipeline->base.handle;
	return handle->base.fpc.color;
}
void setPanelPipelineColor(
	GraphicsPipeline panelPipeline,
	LinearColor color)
{
	assert(panelPipeline);
	assert(strcmp(panelPipeline->base.name,
		PANEL_PIPELINE_NAME) == 0);
	Handle handle = panelPipeline->base.handle;
	handle->base.fpc.color = cmmtColorToVec4(color);
}
