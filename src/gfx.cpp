#include "gfx.h"

#include <SDL3/SDL.h>

#include <math.h>

#include <tinydef.hpp>

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


Gfx::Gfx(SDL_Window* window)
	: windowPtr(window) {}

// NOTE(sand): Right now, we're just batching 6 vertices per quad (32 bytes per quad),
// which means that given 4 megabytes of vertex buffer space the max number of sprites/quads,
// will be (1024 * 1024 * 4) / (32 * 6) = a whopping 21845 quads. I don't think we'll hit this limit :)
//
// If we really want to take it to the next level, we can instance a single unit quad, and
// upload a minified struct of around 12 bytes per quad, which will bring this 21845 much higher.
bool Gfx::init() {
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
			.width = 4096,
			.height = 4096,
			.num_levels = 0,
			.sample_count = SDL_GPU_SAMPLECOUNT_1,
		}; textureAtlas = SDL_CreateGPUTexture(device, &ciTextureAtlas);

		SDL_GPUTextureCreateInfo ciTextureScreen1 = {
			.width = nesWidth,
			.height = nesHeight,
			.num_levels = 0,
			.sample_count = SDL_GPU_SAMPLECOUNT_1,
		}; textureScreen1 = SDL_CreateGPUTexture(device, &ciTextureScreen1);

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

	SDL_Surface* surfaceAtlas = SDL_CreateSurface(4096, 4096, SDL_PIXELFORMAT_RGBA8888);
	// load texture into surface atlas

	textureAtlas = SDL_CreateTextureFromSurface(renderer, surfaceAtlas);
	SDL_DestroySurface(surfaceAtlas);

	textureScreen1 = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, nesWidth, nesHeight);

	SDL_SetTextureBlendMode(textureAtlas, SDL_BLENDMODE_BLEND_PREMULTIPLIED);
	SDL_SetTextureBlendMode(textureScreen1, SDL_BLENDMODE_BLEND_PREMULTIPLIED);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND_PREMULTIPLIED);

	SDL_SetTextureScaleMode(textureAtlas, SDL_SCALEMODE_NEAREST);
	SDL_SetTextureScaleMode(textureScreen1, SDL_SCALEMODE_NEAREST);

#endif

	return true;
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
#ifndef USE_SDL_RENDERER
#else
	SDL_SetRenderTarget(renderer, textureScreen1);
	SDL_SetRenderDrawColorFloat(renderer, EXPAND_COL(clearColor));
	SDL_RenderClear(renderer);
#endif
}

void Gfx::finish_frame() {
	int width = -1, height = -1;
	SDL_GetWindowSize(windowPtr, &width, &height);

#ifndef USE_SDL_RENDERER
	//
	// SDL GPU
	//
	SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(device);
	SDL_GPUColorTargetInfo ctiObject = {
		.texture = textureScreen1,
		.clear_color = { 0.0f, 0.0f, 0.0f, 1.0f },
		.load_op = SDL_GPU_LOADOP_CLEAR,
		.store_op = SDL_GPU_STOREOP_STORE
	};

	// render to nes target
	//SDL_BeginGPURenderPass(commandBuffer, &ctiObject, 1,)

	// render to window swapchain

#else

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

void Gfx::queue_rect(const SDL_FRect& dest, const SDL_FRect& src, const SDL_FColor& color) {
	SDL_SetTextureColorModFloat(textureAtlas, color.r, color.g, color.b);
	SDL_SetTextureAlphaModFloat(textureAtlas, color.a);
	SDL_RenderTexture(renderer, textureAtlas, &src, &dest);
}

void Gfx::queue_rect(const SDL_FRect& dest, const SDL_FColor& color) {
	SDL_SetRenderDrawColorFloat(renderer, EXPAND_COL(color));
	SDL_RenderFillRect(renderer, &dest);
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
	SDL_FColor color;

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