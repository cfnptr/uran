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

#include "uran/pipelines/diffuse_pipeline.h"
#include "mpgx/_source/window.h"
#include "mpgx/_source/graphics_pipeline.h"

#include <string.h>

typedef struct VertexPushConstants
{
	mat4 mvp;
	mat4 normal;
} VertexPushConstants;
typedef struct UniformBuffer
{
	vec4 objectColor;
	vec4 ambientColor;
	vec4 lightColor;
	vec4 lightDirection;
} UniformBuffer;
typedef struct BaseHandle
{
	Logger logger;
	VertexPushConstants vpc;
	UniformBuffer ub;
} BaseHandle;
#if MPGX_SUPPORT_VULKAN
typedef struct VkHandle
{
	Logger logger;
	VertexPushConstants vpc;
	UniformBuffer ub;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	Buffer* uniformBuffers;
	VkDescriptorSet* descriptorSets;
	uint32_t bufferCount;
} VkHandle;
#endif
#if MPGX_SUPPORT_OPENGL
typedef struct GlHandle
{
	Logger logger;
	VertexPushConstants vpc;
	UniformBuffer ub;
	GLint mvpLocation;
	GLint normalLocation;
	Buffer uniformBuffer;
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
		sizeof(Vec3F) * 2,
		VK_VERTEX_INPUT_RATE_VERTEX,
	},
};
static const VkVertexInputAttributeDescription vertexInputAttributeDescriptions[2] = {
	{
		0,
		0,
		VK_FORMAT_R32G32B32_SFLOAT,
		0,
	},
	{
		1,
		0,
		VK_FORMAT_R32G32B32_SFLOAT,
		sizeof(Vec3F),
	},
};
static const VkPushConstantRange pushConstantRanges[1] = {
	{
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		sizeof(VertexPushConstants),
	},
};

inline static MpgxResult createVkDescriptorPoolInstance(
	VkDevice device,
	uint32_t bufferCount,
	VkDescriptorPool* descriptorPool)
{
	assert(device);
	assert(bufferCount > 0);
	assert(descriptorPool);

	VkDescriptorPoolSize descriptorPoolSizes[1] = {
		{
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			bufferCount,
		},
	};
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		NULL,
		0,
		bufferCount,
		1,
		descriptorPoolSizes
	};

	VkDescriptorPool descriptorPoolInstance;

	VkResult vkResult = vkCreateDescriptorPool(
		device,
		&descriptorPoolCreateInfo,
		NULL,
		&descriptorPoolInstance);

	if (vkResult != VK_SUCCESS)
		return vkToMpgxResult(vkResult);

	*descriptorPool = descriptorPoolInstance;
	return SUCCESS_MPGX_RESULT;
}
inline static void destroyVkUniformBuffers(
	uint32_t bufferCount,
	Buffer* uniformBuffers)
{
	assert(bufferCount == 0 ||
		(bufferCount > 0 && uniformBuffers));

	for (uint32_t i = 0; i < bufferCount; i++)
		destroyBuffer(uniformBuffers[i]);

	free(uniformBuffers);
}
inline static MpgxResult createVkUniformBufferArray(
	Window window,
	uint32_t bufferCount,
	Buffer** buffers)
{
	assert(window);
	assert(bufferCount > 0);
	assert(buffers);

	Buffer* bufferArray = malloc(
		bufferCount * sizeof(Buffer));

	if (!bufferArray)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	for (uint32_t i = 0; i < bufferCount; i++)
	{
		Buffer buffer;

		MpgxResult mpgxResult = createBuffer(
			window,
			UNIFORM_BUFFER_TYPE,
			CPU_TO_GPU_BUFFER_USAGE,
			NULL,
			sizeof(UniformBuffer),
			&buffer);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			destroyVkUniformBuffers(
				i, bufferArray);
			return mpgxResult;
		}

		bufferArray[i] = buffer;
	}

	*buffers = bufferArray;
	return SUCCESS_MPGX_RESULT;
}
inline static MpgxResult createVkDescriptorSetArray(
	VkDevice device,
	VkDescriptorSetLayout descriptorSetLayout,
	VkDescriptorPool descriptorPool,
	uint32_t bufferCount,
	Buffer* uniformBuffers,
	VkDescriptorSet** descriptorSets)
{
	assert(device);
	assert(descriptorSetLayout);
	assert(descriptorPool);
	assert(bufferCount > 0);
	assert(uniformBuffers);
	assert(descriptorSets);

	VkDescriptorSet* descriptorSetArray;

	MpgxResult mpgxResult = allocateVkDescriptorSets(
		device,
		descriptorSetLayout,
		descriptorPool,
		bufferCount,
		&descriptorSetArray);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	for (uint32_t i = 0; i < bufferCount; i++)
	{
		VkDescriptorBufferInfo descriptorBufferInfos[1] = {
			{
				uniformBuffers[i]->vk.handle,
				0,
				sizeof(UniformBuffer),
			},
		};
		VkWriteDescriptorSet writeDescriptorSets[1] = {
			{
				VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				NULL,
				descriptorSetArray[i],
				0,
				0,
				1,
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				NULL,
				descriptorBufferInfos,
				NULL,
			}
		};

		vkUpdateDescriptorSets(
			device,
			1,
			writeDescriptorSets,
			0,
			NULL);
	}

	*descriptorSets = descriptorSetArray;
	return SUCCESS_MPGX_RESULT;
}

static void onVkBind(GraphicsPipeline graphicsPipeline)
{
	assert(graphicsPipeline);
	Handle handle = graphicsPipeline->vk.handle;
	VkWindow vkWindow = getVkWindow(graphicsPipeline->vk.window);
	uint32_t bufferIndex = vkWindow->bufferIndex;
	Buffer buffer = handle->vk.uniformBuffers[bufferIndex];

	setVkBufferData(
		vkWindow->allocator,
		buffer->vk.allocation,
		&handle->vk.ub,
		sizeof(UniformBuffer),
		0);
	vkCmdBindDescriptorSets(
		vkWindow->currenCommandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		graphicsPipeline->vk.layout,
		0,
		1,
		&handle->vk.descriptorSets[bufferIndex],
		0,
		NULL);
}
static void onVkUniformsSet(GraphicsPipeline graphicsPipeline)
{
	assert(graphicsPipeline);
	Handle handle = graphicsPipeline->vk.handle;
	VkWindow vkWindow = getVkWindow(graphicsPipeline->vk.window);

	vkCmdPushConstants(
		vkWindow->currenCommandBuffer,
		graphicsPipeline->vk.layout,
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		sizeof(VertexPushConstants),
		&handle->vk.vpc);
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

	Handle handle = graphicsPipeline->vk.handle;
	Window window = graphicsPipeline->vk.window;
	VkWindow vkWindow = getVkWindow(window);
	uint32_t bufferCount = vkWindow->swapchain->bufferCount;

	if (bufferCount != handle->vk.bufferCount)
	{
		VkDevice device = vkWindow->device;

		VkDescriptorPool descriptorPool;

		MpgxResult mpgxResult = createVkDescriptorPoolInstance(
			device,
			bufferCount,
			&descriptorPool);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			if (handle->base.logger)
			{
				logMessage(handle->base.logger, ERROR_LOG_LEVEL,
					"Failed to create descriptor pool. (error: %s)",
					mpgxResultToString(mpgxResult));
			}
			abort();
		}

		Buffer* uniformBuffers;

		mpgxResult = createVkUniformBufferArray(
			window,
			bufferCount,
			&uniformBuffers);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			if (handle->base.logger)
			{
				logMessage(handle->base.logger, ERROR_LOG_LEVEL,
					"Failed to create uniform buffers. (error: %s)",
					mpgxResultToString(mpgxResult));
			}
			abort();
		}

		VkDescriptorSet* descriptorSets;

		mpgxResult = createVkDescriptorSetArray(
			device,
			handle->vk.descriptorSetLayout,
			descriptorPool,
			bufferCount,
			uniformBuffers,
			&descriptorSets);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			if (handle->base.logger)
			{
				logMessage(handle->base.logger, ERROR_LOG_LEVEL,
					"Failed to create descriptor sets. (error: %s)",
					mpgxResultToString(mpgxResult));
			}
			abort();
		}

		free(handle->vk.descriptorSets);

		destroyVkUniformBuffers(
			handle->vk.bufferCount,
			handle->vk.uniformBuffers);
		vkDestroyDescriptorPool(
			device,
			handle->vk.descriptorPool,
			NULL);

		handle->vk.descriptorPool = descriptorPool;
		handle->vk.uniformBuffers = uniformBuffers;
		handle->vk.descriptorSets = descriptorSets;
		handle->vk.bufferCount = bufferCount;
	}

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
		2,
		vertexInputAttributeDescriptions,
		1,
		&handle->vk.descriptorSetLayout,
		1,
		pushConstantRanges,
	};

	*(VkGraphicsPipelineCreateData*)vkCreateData = createData;
}
static void onVkDestroy(
	Window window,
	void* _handle)
{
	assert(window);
	Handle handle = _handle;

	if (!handle)
		return;

	VkWindow vkWindow = getVkWindow(window);
	VkDevice device = vkWindow->device;

	free(handle->vk.descriptorSets);
	destroyVkUniformBuffers(
		handle->vk.bufferCount,
		handle->vk.uniformBuffers);
	vkDestroyDescriptorPool(
		device,
		handle->vk.descriptorPool,
		NULL);
	vkDestroyDescriptorSetLayout(
		device,
		handle->vk.descriptorSetLayout,
		NULL);
	free(handle);
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

	VkWindow vkWindow = getVkWindow(window);
	VkDevice device = vkWindow->device;

	VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[1] = {
		{
			0,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			NULL,
		},
	};
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		NULL,
		0,
		1,
		descriptorSetLayoutBindings
	};

	VkDescriptorSetLayout descriptorSetLayout;

	VkResult vkResult = vkCreateDescriptorSetLayout(
		device,
		&descriptorSetLayoutCreateInfo,
		NULL,
		&descriptorSetLayout);

	if(vkResult != VK_SUCCESS)
	{
		onVkDestroy(window, handle);
		return vkToMpgxResult(vkResult);
	}

	handle->vk.descriptorSetLayout = descriptorSetLayout;

	VkGraphicsPipelineCreateData createData = {
		1,
		vertexInputBindingDescriptions,
		2,
		vertexInputAttributeDescriptions,
		1,
		&descriptorSetLayout,
		1,
		pushConstantRanges,
	};

	uint32_t bufferCount = vkWindow->swapchain->bufferCount;

	VkDescriptorPool descriptorPool;

	MpgxResult mpgxResult = createVkDescriptorPoolInstance(
		device,
		bufferCount,
		&descriptorPool);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		onVkDestroy(window, handle);
		return mpgxResult;
	}

	handle->vk.descriptorPool = descriptorPool;

	Buffer* uniformBuffers;

	mpgxResult = createVkUniformBufferArray(
		window,
		bufferCount,
		&uniformBuffers);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		onVkDestroy(window, handle);
		return mpgxResult;
	}

	handle->vk.uniformBuffers = uniformBuffers;
	handle->vk.bufferCount = bufferCount;

	VkDescriptorSet* descriptorSets;

	mpgxResult = createVkDescriptorSetArray(
		device,
		descriptorSetLayout,
		descriptorPool,
		bufferCount,
		uniformBuffers,
		&descriptorSets);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		onVkDestroy(window, handle);
		return mpgxResult;
	}

	handle->vk.descriptorSets = descriptorSets;

	mpgxResult = createGraphicsPipeline(
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
	Buffer uniformBuffer = handle->gl.uniformBuffer;

	setGlBufferData(
		uniformBuffer->gl.glType,
		uniformBuffer->gl.handle,
		&handle->gl.ub,
		sizeof(UniformBuffer),
		0);

	glBindBufferBase(
		GL_UNIFORM_BUFFER,
		0,
		uniformBuffer->gl.handle);
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
	glUniformMatrix4fv(
		handle->gl.normalLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&handle->gl.vpc.normal);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec3F) * 2,
		0);
	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec3F) * 2,
		(const void*)sizeof(Vec3F));

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
	void* _handle)
{
	assert(window);
	Handle handle = _handle;

	if (!handle)
		return;

	destroyBuffer(handle->gl.uniformBuffer);
	free(handle);
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

	Buffer uniformBuffer;

	MpgxResult mpgxResult = createBuffer(
		window,
		UNIFORM_BUFFER_TYPE,
		CPU_TO_GPU_BUFFER_USAGE,
		NULL,
		sizeof(UniformBuffer),
		&uniformBuffer);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		onGlDestroy(window, handle);
		return mpgxResult;
	}

	handle->gl.uniformBuffer = uniformBuffer;

	GraphicsPipeline graphicsPipelineInstance;

	mpgxResult = createGraphicsPipeline(
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

	GLint mvpLocation, normalLocation;
	GLuint uniformBlockIndex;

	bool result = getGlUniformLocation(
		glHandle,
		"u_MVP",
		&mvpLocation);
	result &= getGlUniformLocation(
		glHandle,
		"u_Normal",
		&normalLocation);
	result &= getGlUniformBlockIndex(
		glHandle,
		"UniformBuffer",
		&uniformBlockIndex);

	if (!result)
	{
		destroyGraphicsPipeline(graphicsPipelineInstance);
		return BAD_SHADER_CODE_MPGX_RESULT;
	}

	glUniformBlockBinding(
		glHandle,
		uniformBlockIndex,
		0);

	assertOpenGL();

	handle->gl.mvpLocation = mvpLocation;
	handle->gl.normalLocation = normalLocation;

	*graphicsPipeline = graphicsPipelineInstance;
	return SUCCESS_MPGX_RESULT;
}
#endif

MpgxResult createDiffusePipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	const GraphicsPipelineState* state,
	Logger logger,
	GraphicsPipeline* diffusePipeline)
{
	assert(framebuffer);
	assert(vertexShader);
	assert(fragmentShader);
	assert(diffusePipeline);
	assert(vertexShader->base.type == VERTEX_SHADER_TYPE);
	assert(fragmentShader->base.type == FRAGMENT_SHADER_TYPE);
	assert(vertexShader->base.window == framebuffer->base.window);
	assert(fragmentShader->base.window == framebuffer->base.window);

	Handle handle = calloc(1, sizeof(Handle_T));

	if (!handle)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	Vec3F lightDirection = normVec3F(
		vec3F(1.0f, -3.0f, 6.0f));

	UniformBuffer ub = {
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ 0.5f, 0.5f, 0.5f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		{ lightDirection.x, lightDirection.y, lightDirection.z, 0.0f },
	};

	handle->base.logger = logger;
	handle->base.ub = ub;

#ifndef NDEBUG
	const char* name = DIFFUSE_PIPELINE_NAME;
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
		ZERO_BLEND_FACTOR,
		ZERO_BLEND_FACTOR,
		ZERO_BLEND_FACTOR,
		ZERO_BLEND_FACTOR,
		ADD_BLEND_OPERATOR,
		ADD_BLEND_OPERATOR,
		true,
		true,
		true,
		true,
		false,
		false,
		false,
		false,
		false,
		DEFAULT_LINE_WIDTH,
		size,
		size,
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
			diffusePipeline);
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
			diffusePipeline);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}
}

const mat4* getDiffusePipelineMvp(
	GraphicsPipeline diffusePipeline)
{
	assert(diffusePipeline);
	assert(strcmp(diffusePipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	Handle handle = diffusePipeline->base.handle;
	return &handle->base.vpc.mvp;
}
void setDiffusePipelineMvp(
	GraphicsPipeline diffusePipeline,
	const Mat4F* mvp)
{
	assert(diffusePipeline);
	assert(mvp);
	assert(strcmp(diffusePipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	Handle handle = diffusePipeline->base.handle;
	handle->base.vpc.mvp = cmmtToMat4(*mvp);
}

const mat4* getDiffusePipelineNormal(
	GraphicsPipeline diffusePipeline)
{
	assert(diffusePipeline);
	assert(strcmp(diffusePipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	Handle handle = diffusePipeline->base.handle;
	return &handle->base.vpc.normal;
}
void setDiffusePipelineNormal(
	GraphicsPipeline diffusePipeline,
	const Mat4F* normal)
{
	assert(diffusePipeline);
	assert(normal);
	assert(strcmp(diffusePipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	Handle handle = diffusePipeline->base.handle;
	handle->base.vpc.normal = cmmtToMat4(*normal);
}

vec4 getDiffusePipelineObjectColor(
	GraphicsPipeline diffusePipeline)
{
	assert(diffusePipeline);
	assert(strcmp(diffusePipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	Handle handle = diffusePipeline->base.handle;
	return handle->base.ub.objectColor;
}
void setDiffusePipelineObjectColor(
	GraphicsPipeline diffusePipeline,
	LinearColor objectColor)
{
	assert(diffusePipeline);
	assert(strcmp(diffusePipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	Handle handle = diffusePipeline->base.handle;
	handle->base.ub.objectColor = cmmtColorToVec4(objectColor);
}

vec4 getDiffusePipelineAmbientColor(
	GraphicsPipeline diffusePipeline)
{
	assert(diffusePipeline);
	assert(strcmp(diffusePipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	Handle handle = diffusePipeline->base.handle;
	return handle->base.ub.ambientColor;
}
void setDiffusePipelineAmbientColor(
	GraphicsPipeline diffusePipeline,
	LinearColor ambientColor)
{
	assert(diffusePipeline);
	assert(strcmp(diffusePipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	Handle handle = diffusePipeline->base.handle;
	handle->base.ub.ambientColor = cmmtColorToVec4(ambientColor);
}

vec4 getDiffusePipelineLightColor(
	GraphicsPipeline diffusePipeline)
{
	assert(diffusePipeline);
	assert(strcmp(diffusePipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	Handle handle = diffusePipeline->base.handle;
	return handle->base.ub.lightColor;
}
void setDiffusePipelineLightColor(
	GraphicsPipeline diffusePipeline,
	LinearColor lightColor)
{
	assert(diffusePipeline);
	assert(strcmp(diffusePipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	Handle handle = diffusePipeline->base.handle;
	handle->base.ub.lightColor = cmmtColorToVec4(lightColor);
}

vec4 getDiffusePipelineLightDirection(
	GraphicsPipeline diffusePipeline)
{
	assert(diffusePipeline);
	assert(strcmp(diffusePipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	Handle handle = diffusePipeline->base.handle;
	return handle->base.ub.lightDirection;
}
void setDiffusePipelineLightDirection(
	GraphicsPipeline diffusePipeline,
	Vec3F lightDirection)
{
	assert(diffusePipeline);
	assert(strcmp(diffusePipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	Handle handle = diffusePipeline->base.handle;
	Vec3F direction = normVec3F(lightDirection);

	vec4 vector = {
		direction.x, direction.y, direction.z, 0.0f,
	};

	handle->base.ub.lightDirection = vector;
}
