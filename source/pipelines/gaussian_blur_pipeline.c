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

#include "uran/pipelines/gaussian_blur_pipeline.h"
#include "mpgx/_source/window.h"
#include "mpgx/_source/graphics_pipeline.h"
#include "mpgx/_source/sampler.h"

#include <string.h>

typedef struct FragmentPushConstants
{
	int radius;
	int offset;
} FragmentPushConstants;
typedef struct BaseHandle
{
	Image buffer;
	Sampler sampler;
	FragmentPushConstants fpc;
} BaseHandle;
#if MPGX_SUPPORT_VULKAN
typedef struct VkHandle
{
	Image buffer;
	Sampler sampler;
	FragmentPushConstants fpc;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSet;
} VkHandle;
#endif
#if MPGX_SUPPORT_OPENGL
typedef struct GlHandle
{
	Image buffer;
	Sampler sampler;
	FragmentPushConstants fpc;
	GLint radiusLocation;
	GLint offsetLocation;
	GLint bufferLocation;
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
		sizeof(Vec2F) * 2,
		VK_VERTEX_INPUT_RATE_VERTEX,
	},
};
static const VkVertexInputAttributeDescription vertexInputAttributeDescriptions[2] = {
	{
		0,
		0,
		VK_FORMAT_R32G32_SFLOAT,
		0,
	},
	{
		1,
		0,
		VK_FORMAT_R32G32_SFLOAT,
		sizeof(Vec2F),
	},
};
static const VkPushConstantRange pushConstantRanges[1] = {
	{
		VK_SHADER_STAGE_FRAGMENT_BIT,
		0,
		sizeof(FragmentPushConstants),
	},
};

inline static MpgxResult createVkDescriptorPoolInstance(
	VkDevice device,
	VkDescriptorPool* descriptorPool)
{
	assert(device);
	assert(descriptorPool);

	VkDescriptorPoolSize descriptorPoolSizes[1] = {
		{
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			1,
		},
	};

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		NULL,
		0,
		1,
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
inline static MpgxResult createVkDescriptorSetInstance(
	VkDevice device,
	VkDescriptorSetLayout descriptorSetLayout,
	VkDescriptorPool descriptorPool,
	VkImageView imageView,
	VkSampler sampler,
	VkDescriptorSet* descriptorSet)
{
	assert(device);
	assert(descriptorSetLayout);
	assert(descriptorPool);
	assert(imageView);
	assert(sampler);
	assert(descriptorSet);

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		NULL,
		descriptorPool,
		1,
		&descriptorSetLayout,
	};

	VkDescriptorSet descriptorSetInstance;

	VkResult vkResult = vkAllocateDescriptorSets(
		device,
		&descriptorSetAllocateInfo,
		&descriptorSetInstance);

	if (vkResult != VK_SUCCESS)
		return vkToMpgxResult(vkResult);

	VkDescriptorImageInfo bufferDescriptorImageInfos[1] = {
		{
			sampler,
			imageView,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		},
	};

	VkWriteDescriptorSet writeDescriptorSets[1] = {
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			NULL,
			descriptorSetInstance,
			0,
			0,
			1,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			bufferDescriptorImageInfos,
			NULL,
			NULL,
		},
	};

	vkUpdateDescriptorSets(
		device,
		1,
		writeDescriptorSets,
		0,
		NULL);

	*descriptorSet = descriptorSetInstance;
	return SUCCESS_MPGX_RESULT;
}

static void onVkBind(GraphicsPipeline graphicsPipeline)
{
	assert(graphicsPipeline);
	Handle handle = graphicsPipeline->vk.handle;
	VkWindow vkWindow = getVkWindow(graphicsPipeline->vk.window);

	vkCmdBindDescriptorSets(
		vkWindow->currenCommandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		graphicsPipeline->vk.layout,
		0,
		1,
		&handle->vk.descriptorSet,
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
		VK_SHADER_STAGE_FRAGMENT_BIT,
		0,
		sizeof(FragmentPushConstants),
		&handle->vk.fpc);
}
static void onVkResize(
	GraphicsPipeline graphicsPipeline,
	Vec2I newSize,
	void* createData)
{
	assert(graphicsPipeline);
	assert(newSize.x > 0);
	assert(newSize.y > 0);
	assert(createData);

	Handle handle = graphicsPipeline->vk.handle;

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

	VkGraphicsPipelineCreateData _createData = {
		1,
		vertexInputBindingDescriptions,
		2,
		vertexInputAttributeDescriptions,
		1,
		&handle->vk.descriptorSetLayout,
		1,
		pushConstantRanges,
	};

	*(VkGraphicsPipelineCreateData*)createData = _createData;
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
	VkImageView imageView,
	VkSampler sampler,
	const GraphicsPipelineState* state,
	Handle handle,
	Shader* shaders,
	uint8_t shaderCount,
	GraphicsPipeline* graphicsPipeline)
{
	assert(framebuffer);
	assert(window);
	assert(imageView);
	assert(sampler);
	assert(state);
	assert(handle);
	assert(shaders);
	assert(shaderCount > 0);
	assert(graphicsPipeline);

	VkWindow vkWindow = getVkWindow(framebuffer->vk.window);
	VkDevice device = vkWindow->device;

	VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[1] = {
		{
			0,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
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

	VkDescriptorPool descriptorPool;

	MpgxResult mpgxResult = createVkDescriptorPoolInstance(
		device,
		&descriptorPool);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		onVkDestroy(window, handle);
		return mpgxResult;
	}

	handle->vk.descriptorPool = descriptorPool;

	VkDescriptorSet descriptorSet;

	mpgxResult = createVkDescriptorSetInstance(
		device,
		descriptorSetLayout,
		descriptorPool,
		imageView,
		sampler,
		&descriptorSet);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		onVkDestroy(window, handle);
		return mpgxResult;
	}

	handle->vk.descriptorSet = descriptorSet;

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

	glUniform1i(handle->gl.bufferLocation, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, handle->gl.buffer->gl.handle);
	glBindSampler(0, handle->gl.sampler->gl.handle);

	assertOpenGL();
}
static void onGlUniformsSet(GraphicsPipeline graphicsPipeline)
{
	assert(graphicsPipeline);
	Handle handle = graphicsPipeline->gl.handle;

	glUniform1iv(
		handle->gl.radiusLocation,
		1,
		(const GLint*)&handle->gl.fpc.radius);
	glUniform1iv(
		handle->gl.radiusLocation,
		1,
		(const GLint*)&handle->gl.fpc.offset);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(
		0,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec2F) * 2,
		0);
	glVertexAttribPointer(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec2F) * 2,
		(const void*)sizeof(Vec2F));

	assertOpenGL();
}
static void onGlResize(
	GraphicsPipeline graphicsPipeline,
	Vec2I newSize,
	void* createData)
{
	assert(graphicsPipeline);
	assert(newSize.x > 0);
	assert(newSize.y > 0);
	assert(!createData);

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
		NULL,
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

	GLint radiusLocation,
		offsetLocation,
		bufferLocation;

	bool result = getGlUniformLocation(
		glHandle,
		"u_Radius",
		&radiusLocation);
	result &= getGlUniformLocation(
		glHandle,
		"u_Offset",
		&offsetLocation);
	result &= getGlUniformLocation(
		glHandle,
		"u_Buffer",
		&bufferLocation);

	if (!result)
	{
		destroyGraphicsPipeline(graphicsPipelineInstance);
		return BAD_SHADER_CODE_MPGX_RESULT;
	}

	assertOpenGL();

	handle->gl.radiusLocation = radiusLocation;
	handle->gl.offsetLocation = offsetLocation;
	handle->gl.bufferLocation = bufferLocation;

	*graphicsPipeline = graphicsPipelineInstance;
	return SUCCESS_MPGX_RESULT;
}
#endif

inline static int calcGaussianOffset(int radius)
{
	int offset = 0;
	for (int i = 0; i < radius; i++)
		offset += i + 1;
	return offset;
}

MpgxResult createGaussianBlurPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image buffer,
	Sampler sampler,
	const GraphicsPipelineState* state,
	GraphicsPipeline* gaussianBlurPipeline)
{
	assert(framebuffer);
	assert(vertexShader);
	assert(fragmentShader);
	assert(buffer);
	assert(sampler);
	assert(gaussianBlurPipeline);
	assert(vertexShader->base.type == VERTEX_SHADER_TYPE);
	assert(fragmentShader->base.type == FRAGMENT_SHADER_TYPE);
	assert(vertexShader->base.window == framebuffer->base.window);
	assert(fragmentShader->base.window == framebuffer->base.window);
	assert(buffer->base.window == framebuffer->base.window);
	assert(sampler->base.window == framebuffer->base.window);

	Handle handle = calloc(1, sizeof(Handle_T));

	if (!handle)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	handle->base.buffer = buffer;
	handle->base.sampler = sampler;
	handle->base.fpc.radius = 8;
	handle->base.fpc.offset = calcGaussianOffset(8);

#ifndef NDEBUG
	const char* name = GAUSSIAN_BLUR_PIPELINE_NAME;
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
		false,
		false,
		false,
		false,
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
			buffer->vk.imageView,
			sampler->vk.handle,
			state ? state : &defaultState,
			handle,
			shaders,
			2,
			gaussianBlurPipeline);
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
			gaussianBlurPipeline);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}
}

Image getGaussianBlurPipelineBuffer(
	GraphicsPipeline gaussianBlurPipeline)
{
	assert(gaussianBlurPipeline);
	assert(strcmp(gaussianBlurPipeline->base.name,
		GAUSSIAN_BLUR_PIPELINE_NAME) == 0);
	Handle handle = gaussianBlurPipeline->base.handle;
	return handle->base.buffer;
}
Sampler getGaussianBlurPipelineSampler(
	GraphicsPipeline gaussianBlurPipeline)
{
	assert(gaussianBlurPipeline);
	assert(strcmp(gaussianBlurPipeline->base.name,
		GAUSSIAN_BLUR_PIPELINE_NAME) == 0);
	Handle handle = gaussianBlurPipeline->base.handle;
	return handle->base.sampler;
}

int getGaussianBlurPipelineRadius(
	GraphicsPipeline gaussianBlurPipeline)
{
	assert(gaussianBlurPipeline);
	assert(strcmp(gaussianBlurPipeline->base.name,
		GAUSSIAN_BLUR_PIPELINE_NAME) == 0);
	Handle handle = gaussianBlurPipeline->base.handle;
	return handle->base.fpc.radius;
}
void setGaussianBlurPipelineRadius(
	GraphicsPipeline gaussianBlurPipeline,
	int radius)
{
	assert(gaussianBlurPipeline);
	assert(radius >= 0);
	assert(radius <= 16);
	assert(strcmp(gaussianBlurPipeline->base.name,
		GAUSSIAN_BLUR_PIPELINE_NAME) == 0);
	Handle handle = gaussianBlurPipeline->base.handle;
	handle->base.fpc.radius = radius;
	handle->base.fpc.offset = calcGaussianOffset(radius);
}
