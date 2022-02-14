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
#include "uran/graphics_renderer.h"
#include "uran/text.h"

/*
 * Create a new text renderer instance.
 * Returns text renderer instance in success, otherwise NULL.
 *
 * textPipeline - text pipeline instance.
 * sorting - render sorting type.
 * useCulling - use frustum culling.
 * capacity - initial render array capacity.
 * threadPool - thread pool instance or NULL.
 */
GraphicsRenderer createTextRenderer(
	GraphicsPipeline textPipeline,
	GraphicsRenderSorting sorting,
	bool useCulling,
	size_t capacity,
	ThreadPool threadPool);

/*
 * Create a new text render instance.
 * Returns text render instance on success, otherwise NULL.
 *
 * textRenderer - text renderer instance.
 * transform - transform instance.
 * bounds - render bounds.
 * color - render color.
 * text - render text.
 * scissor - scissor or zero vector.
 */
GraphicsRender createTextRender(
	GraphicsRenderer textRenderer,
	Transform transform,
	Box3F bounds,
	LinearColor color,
	Text text,
	Vec4I scissor);

/*
 * Returns text render color.
 * textRender - text render instance.
 */
LinearColor getTextRenderColor(
	GraphicsRender textRender);
/*
 * Sets text render color.
 *
 * textRender - text render instance.
 * color - color value.
 */
void setTextRenderColor(
	GraphicsRender textRender,
	LinearColor color);

/*
 * Returns text render scissor.
 * textRender - text render instance.
 */
Vec4I getTextRenderScissor(
	GraphicsRender textRender);
/*
 * Sets text render scissor.
 *
 * textRender - text render instance.
 * scissor - scissor value or zero vector.
 */
void setTextRenderScissor(
	GraphicsRender textRender,
	Vec4I scissor);

/*
 * Returns text render text.
 * textRender - text render instance.
 */
Text getTextRenderText(
	GraphicsRender textRender);
/*
 * Sets text render text.
 *
 * textRender - text render instance.
 * text - text instance.
 */
void setTextRenderText(
	GraphicsRender textRender,
	Text text);
