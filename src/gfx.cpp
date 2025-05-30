#include "gfx.h"

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
		return size_quad() * (nesWidth / 16) * (nesHeight / 16);
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

	// LEFT OFF HERE
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

	return true;
}

void Gfx::cleanup() {
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
}

void Gfx::finish_frame() {

}