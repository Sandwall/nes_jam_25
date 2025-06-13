#include "gfx.h"

#include <SDL3/SDL.h>

#include <math.h>

#include <tinydef.hpp>

#include "engine/image_asset.h"

#pragma warning (disable : 4101)

// NOTE(sand): This is a struct that represents a single vertex
// I primarily made it for the constexpr size functions that change with the size of the struct
struct AttributedVertex {
	static constexpr int size() {
		return sizeof(AttributedVertex);
	}

	static constexpr int size_tri() {
		return size() * 3;
	}

	static constexpr int size_quad() {
		return size() * 6;
	}

	static constexpr int size_tile_layer() {
		return size_quad() * (Gfx::nesWidth / 16) * (Gfx::nesHeight / 16);
	}

	float x, y, depth;    // vertex position
	float u, v;           // texture coord
	float r, g, b;        // vertex color
};

constexpr int FOUR_MEGS = 1024 * 1024 * 4;

Gfx::Gfx() {}

// NOTE(sand): Right now, we're just batching 6 vertices per quad (32 bytes per quad),
// which means that given 4 megabytes of vertex buffer space the max number of sprites/quads,
// will be (1024 * 1024 * 4) / (32 * 6) = a whopping 21845 quads. I don't think we'll hit this limit :)
//
// If we really want to take it to the next level, we can instance a single unit quad, and
// upload a minified struct of around 12 bytes per quad, which will bring this 21845 much higher.
bool Gfx::init(SDL_Window* window) {
	windowPtr = window;

#ifndef USE_SDL_RENDERER
	//
	// SDL_GPU
	//

	// init gpu
	device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, "vulkan");
	if (!SDL_ClaimWindowForGPUDevice(device, windowPtr)) return false;

	// init textures
	{
		SDL_GPUTextureCreateInfo ciTextureAtlas = {
			.type = SDL_GPU_TEXTURETYPE_2D,
			.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UINT,
			.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
			.width = 4096,
			.height = 4096,
			.num_levels = 0,
			.sample_count = SDL_GPU_SAMPLECOUNT_1,
		}; textureAtlas = SDL_CreateGPUTexture(device, &ciTextureAtlas);

		SDL_GPUTextureCreateInfo ciTextureScreen1 = {
			.type = SDL_GPU_TEXTURETYPE_2D,
			.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET,    // if we get an error here, we may want to OR SDL_GPU_TEXTUREUSAGE_SAMPLER
			.width = nesWidth,
			.height = nesHeight,
			.num_levels = 0,
			.sample_count = SDL_GPU_SAMPLECOUNT_1,
		}; textureScreen1 = SDL_CreateGPUTexture(device, &ciTextureScreen1);

		SDL_GPUTextureCreateInfo ciDepthStencil = {
			.type = SDL_GPU_TEXTURETYPE_2D,
			.format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
			.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
			.width = nesWidth,
			.height = nesHeight,
			.num_levels = 0,
			.sample_count = SDL_GPU_SAMPLECOUNT_1,
		};

		ctiObject = {
			.texture = textureScreen1,
			.clear_color = { 0.0f, 0.0f, 0.0f, 1.0f },
			.load_op = SDL_GPU_LOADOP_CLEAR,
			.store_op = SDL_GPU_STOREOP_STORE
		};

		dstiObject = {
			.texture = textureDepth,
			.clear_depth = 0.0f,
			.load_op = SDL_GPU_LOADOP_CLEAR,
			.store_op = SDL_GPU_STOREOP_DONT_CARE,            // only care about depth during the render pass
			.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE,
			.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE,
			.clear_stencil = 0,
		};

		// Around this point, we probably want to build our texture atlas
		// I'm not sure how we'll handle this, maybe we'll find some way to do it in aseprite
		// or use stb_rect_pack to do it on the fly with a list of PNGs
	}

	// init shaders
	{
		// load the shaders from files
		// TODO(sand):

		// actually create the shaders
		SDL_GPUShaderCreateInfo ciVertShaderObject = {
		}; vertShaderObject = SDL_CreateGPUShader(device, &ciVertShaderObject);

		SDL_GPUShaderCreateInfo ciFragShaderObject = {
		}; fragShaderObject = SDL_CreateGPUShader(device, &ciFragShaderObject);

		SDL_GPUShaderCreateInfo ciVertShaderScreen1 = {
		}; vertShaderScreen1 = SDL_CreateGPUShader(device, &ciVertShaderScreen1);

		SDL_GPUShaderCreateInfo ciFragShaderScreen1 = {
		}; fragShaderScreen1 = SDL_CreateGPUShader(device, &ciFragShaderScreen1);
	}

	// init buffers
	{
		SDL_GPUBufferCreateInfo ciBufferObject = {
			.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
			.size = FOUR_MEGS,
		}; bufferObject = SDL_CreateGPUBuffer(device, &ciBufferObject);

		SDL_GPUBufferCreateInfo ciBufferScreen = {
			.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
			.size = AttributedVertex::size_quad()
		}; bufferScreen = SDL_CreateGPUBuffer(device, &ciBufferScreen);
	}

	// init sampler (the default setting is a nearest sampler)
	{
		SDL_GPUSamplerCreateInfo ciSamplerNearest = { };
		samplerNearest = SDL_CreateGPUSampler(device, &ciSamplerNearest);
	}

	// init pipelines
	{
		float x, y, depth;    // vertex position
		float u, v;           // texture coord
		float r, g, b;        // vertex color
		SDL_GPUVertexAttribute attributes[3] = {
			{	// float x, y, depth    for vertex position
				.location = 0,
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
				.offset = 0,
			},
			{	// float u, v           for texture coord
				.location = 0,
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
				.offset = sizeof(float) * 3,
			},
			{	// float r, g, b        for vertex color
				.location = 0,
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
				.offset = sizeof(float) * 5,
			},
		};

		SDL_GPUGraphicsPipelineCreateInfo ciPipelineObject = {
			.vertex_shader = vertShaderObject,
			.fragment_shader = fragShaderObject,
			.vertex_input_state = {
				.num_vertex_buffers = 1,
				.vertex_attributes = attributes,
				.num_vertex_attributes = 3,
			},
			.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
			.rasterizer_state = {
				.cull_mode = SDL_GPU_CULLMODE_BACK,    // there's like 
			}
		}; pipelineObject = SDL_CreateGPUGraphicsPipeline(device, &ciPipelineObject);

		SDL_GPUGraphicsPipelineCreateInfo ciPipelineScreen1 = {
			.vertex_shader = vertShaderScreen1,
			.fragment_shader = fragShaderScreen1,
			.vertex_input_state = {
				.num_vertex_buffers = 1,
				.vertex_attributes = attributes,
				.num_vertex_attributes = 3,
			},
			.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		}; pipelineScreen1 = SDL_CreateGPUGraphicsPipeline(device, &ciPipelineScreen1);
	}

#else
	//
	// SDL_Renderer
	//
	renderer = SDL_CreateRenderer(windowPtr, nullptr);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND_PREMULTIPLIED);

	textureScreen1 = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, nesWidth, nesHeight);
	SDL_SetTextureBlendMode(textureScreen1, SDL_BLENDMODE_BLEND_PREMULTIPLIED);
	SDL_SetTextureScaleMode(textureScreen1, SDL_SCALEMODE_NEAREST);

#endif

	return true;
}

void Gfx::upload_atlas(const TextureAtlas& atlas) {
	spriteAtlas = &atlas;

	// for some reason each pixel is loaded with the endianness swapped
	// maybe we're doing something wrong in the TextureAtlas image loading code?
	// NOTE(sand): setting the pixelformat to ABGR instead of RGBA interprets the pixels properly
	textureAtlas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, atlas.width, atlas.height);

	SDL_UpdateTexture(textureAtlas, nullptr, atlas.data, atlas.width * TextureAtlas::NUM_CHANNELS);

	SDL_SetTextureBlendMode(textureAtlas, SDL_BLENDMODE_BLEND_PREMULTIPLIED);
	SDL_SetTextureScaleMode(textureAtlas, SDL_SCALEMODE_NEAREST);
}

void Gfx::cleanup() {

#ifndef USE_SDL_RENDERER
	//
	// SDL GPU
	//
	SDL_ReleaseGPUGraphicsPipeline(device, pipelineObject);
	SDL_ReleaseGPUGraphicsPipeline(device, pipelineScreen1);

	SDL_ReleaseGPUSampler(device, samplerNearest);

	SDL_ReleaseGPUBuffer(device, bufferObject);
	SDL_ReleaseGPUBuffer(device, bufferScreen);

	SDL_ReleaseGPUShader(device, vertShaderScreen1);
	SDL_ReleaseGPUShader(device, fragShaderObject);
	SDL_ReleaseGPUShader(device, vertShaderScreen1);
	SDL_ReleaseGPUShader(device, fragShaderScreen1);

	SDL_ReleaseGPUTexture(device, textureAtlas);
	SDL_ReleaseGPUTexture(device, textureScreen1);

	SDL_ReleaseWindowFromGPUDevice(device, windowPtr);
	SDL_DestroyGPUDevice(device);
#else
	//
	// SDL Renderer
	//
	SDL_DestroyTexture(textureAtlas);
	SDL_DestroyTexture(textureScreen1);
	SDL_DestroyRenderer(renderer);
#endif
}

void Gfx::begin_frame() {
	assert(spriteAtlas != nullptr);
	assert(fontIdx != UINT32_MAX);

#ifndef USE_SDL_RENDERER
#else
	SDL_SetRenderTarget(renderer, textureScreen1);
	SDL_SetRenderDrawColorFloat(renderer, EXPAND_COL(clearColor));
	SDL_RenderClear(renderer);
#endif
}

void Gfx::finish_frame() {
#ifndef USE_SDL_RENDERER
	//
	// SDL GPU
	//
	SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(device);

	// render to nes target
	SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &ctiObject, 1, &dstiObject);

	SDL_EndGPURenderPass(renderPass);

	// render nes target to swapchain
	SDL_GPUTexture* textureSwapchain = nullptr;
	Uint32 width = -1, height = -1;
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, windowPtr, &textureSwapchain, &width, &height)) {
		fprintf(stderr, "Could not acquire swapchain texture!\n");
		return;
	}

	ctiScreen1 = {
		.texture = textureSwapchain,
		.clear_color = { 0.0f, 0.0f, 0.0f, 1.0f },
		.load_op = SDL_GPU_LOADOP_CLEAR,
		.store_op = SDL_GPU_STOREOP_STORE
	};

	// render to window swapchain
	renderPass = SDL_BeginGPURenderPass(commandBuffer, &ctiScreen1, 1, nullptr);

	SDL_EndGPURenderPass(renderPass);

	SDL_SubmitGPUCommandBuffer(commandBuffer);

#else
	int width = -1, height = -1;
	SDL_GetWindowSize(windowPtr, &width, &height);

	// SDL_Renderer
	SDL_SetRenderTarget(renderer, nullptr);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	SDL_FRect dest = { 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height) };

	// have the image scale to the minimum of the 2 dimensions
	if (width < height) {
		float ratio = static_cast<float>(width) / static_cast<float>(nesWidth);
		
		dest.h = ratio * static_cast<float>(nesHeight);
		dest.y = (static_cast<float>(height) - dest.h) / 2.0f;
	} else {
		float ratio = static_cast<float>(height) / static_cast<float>(nesHeight);

		dest.w = ratio * static_cast<float>(nesWidth);
		dest.x = (static_cast<float>(width) - dest.w) / 2.0f;
	}

	SDL_RenderTexture(renderer, textureScreen1, nullptr, &dest);
	SDL_RenderPresent(renderer);

#endif

}

void Gfx::queue_point(SDL_FPoint pt, bool useCamera, const SDL_FColor& color) {
#ifndef USE_SDL_RENDERER
#else
	if (useCamera) {
		pt.x -= cameraPos.x;
		pt.y -= cameraPos.y;
	}
	SDL_SetRenderDrawColorFloat(renderer, EXPAND_COL(color));
	SDL_RenderPoint(renderer, pt.x, pt.y);
#endif
}

void Gfx::queue_rect(SDL_FRect dest, bool useCamera, const SDL_FColor& color) {
#ifndef USE_SDL_RENDERER
#else
	if (useCamera) {
		dest.x -= cameraPos.x;
		dest.y -= cameraPos.y;
	}
	SDL_SetRenderDrawColorFloat(renderer, EXPAND_COL(color));
	SDL_RenderFillRect(renderer, &dest);
#endif
}

void Gfx::queue_text(int x, int y, const char* text, const SDL_FColor& color) {
	const SubTexture& fontSubtex = spriteAtlas->subTextures[fontIdx];
	size_t len = strlen(text);

	int px = x, py = y;
	for (int i = 0; i < len; i++) {
		switch (text[i]) {
		case '\n':
			px = x;
			py += 8;
			break;
		default:
			int unrolled = (text[i] - 32) * 8;
			SDL_Rect src = {
				unrolled % fontSubtex.width,
				(unrolled / fontSubtex.width) * 8,
				8,
				8
			};
			queue_sprite(px, py, fontSubtex, src, false, color);
			px += 8;
		}
	}
}

void Gfx::queue_sprite(int x, int y, const SubTexture& subTex, const SDL_Rect& src, bool useCamera, const SDL_FColor& color, bool flipH, bool flipV) {
	SDL_SetTextureColorModFloat(textureAtlas, color.r, color.g, color.b);
	SDL_SetTextureAlphaModFloat(textureAtlas, color.a);

	SDL_FRect dest = {
		static_cast<float>(x),
		static_cast<float>(y),
		static_cast<float>(src.w),
		static_cast<float>(src.h)
	};

	SDL_FRect fSrc;
	SDL_RectToFRect(&src, &fSrc);
	fSrc.x += subTex.x;
	fSrc.y += subTex.y;

	if (useCamera) {
		dest.x -= cameraPos.x;
		dest.y -= cameraPos.y;
	}

	SDL_FlipMode flipMode = static_cast<SDL_FlipMode>((flipH ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE) | (flipV ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE));
	SDL_RenderTextureRotated(renderer, textureAtlas, &fSrc, &dest, 0.0, nullptr, flipMode);
}


void Gfx::queue_sprite(int x, int y, uint32_t spriteIdx, const SDL_Rect& src, bool useCamera, const SDL_FColor& color, bool flipH, bool flipV) {
	if (spriteIdx == TextureAtlas::INVALID_IDX)
		return;

	queue_sprite(x, y, spriteAtlas->subTextures[spriteIdx], src, useCamera, color, flipH, flipV);
}

/*
void Gfx::queue_rect(const SDL_FRect& dest, const SDL_FRect& src, const SDL_Color& color) {
	SDL_SetTextureColorModFloat(textureAtlas, color.r, color.g, color.b);
	SDL_SetTextureAlphaModFloat(textureAtlas, color.a);
	SDL_RenderTexture(renderer, textureAtlas, &src, &dest);
}

void Gfx::queue_rect(const SDL_FRect& dest, const SDL_Color& color) {
	SDL_SetRenderDrawColor(renderer, EXPAND_COL(color));
	SDL_RenderRect(renderer, &dest);
}
*/

SDL_FColor Gfx::hsv_to_col(float h, float s, float v, float a) {
	SDL_FColor color = {};

	// Normalize hue to 0-360 range
	h = fmodf(h, 360.0f);
	if (h < 0.0f) h += 360.0f;

	// Clamp input values to valid ranges
	s = tim::clamp(s, 0.0f, 1.0f);
	v = tim::clamp(v, 0.0f, 1.0f);
	a = tim::clamp(a, 0.0f, 1.0f);

	// Handle grayscale case (saturation = 0)
	if (s == 0.0f) {
		color.r = v;
		color.g = v;
		color.b = v;
		color.a = a;
		return color;
	}

	// Convert HSV to RGB
	float c = v * s;  // Chroma
	float h_prime = h / 60.0f;
	float x = c * (1.0f - fabsf(fmodf(h_prime, 2.0f) - 1.0f));
	float m = v - c;

	float r_temp, g_temp, b_temp;

	if (h_prime >= 0.0f && h_prime < 1.0f) {
		r_temp = c;
		g_temp = x;
		b_temp = 0.0f;
	} else if (h_prime >= 1.0f && h_prime < 2.0f) {
		r_temp = x;
		g_temp = c;
		b_temp = 0.0f;
	} else if (h_prime >= 2.0f && h_prime < 3.0f) {
		r_temp = 0.0f;
		g_temp = c;
		b_temp = x;
	} else if (h_prime >= 3.0f && h_prime < 4.0f) {
		r_temp = 0.0f;
		g_temp = x;
		b_temp = c;
	} else if (h_prime >= 4.0f && h_prime < 5.0f) {
		r_temp = x;
		g_temp = 0.0f;
		b_temp = c;
	} else {
		r_temp = c;
		g_temp = 0.0f;
		b_temp = x;
	}

	color.r = r_temp + m;
	color.g = g_temp + m;
	color.b = b_temp + m;
	color.a = a;

	return color;
}

SDL_FPoint Gfx::world_to_screen(SDL_FPoint pt) const {
	return SDL_FPoint{ pt.x - cameraPos.x, pt.y - cameraPos.y };
}

SDL_FPoint Gfx::screen_to_world(SDL_FPoint pt) const {
	return SDL_FPoint{ pt.x + cameraPos.x, pt.y + cameraPos.y };
}

SDL_Rect Gfx::cam_bbox() const {
	return SDL_Rect{ static_cast<int>(cameraPos.x), static_cast<int>(cameraPos.y), nesWidth, nesHeight };
}

SDL_FRect Gfx::cam_bboxf() const {
	return SDL_FRect{ cameraPos.x, cameraPos.y, static_cast<float>(nesWidth), static_cast<float>(nesHeight) };
}
