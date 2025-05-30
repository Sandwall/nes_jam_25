#pragma once

#include <SDL3/SDL_gpu.h>

constexpr int nesWidth = 256, nesHeight = 240;

class Gfx {
	SDL_Window* windowPtr = nullptr;

	SDL_GPUDevice* device = nullptr;

	SDL_GPUTexture* textureAtlas = nullptr;
	SDL_GPUTexture* textureScreen1 = nullptr;

	SDL_GPUShader* vertShaderObject  = nullptr;
	SDL_GPUShader* fragShaderObject  = nullptr;
	SDL_GPUShader* vertShaderScreen1 = nullptr;
	SDL_GPUShader* fragShaderScreen1 = nullptr;

	SDL_GPUBuffer* bufferObject = nullptr;
	SDL_GPUBuffer* bufferScreen = nullptr;

	SDL_GPUSampler* samplerNearest = nullptr;

	SDL_GPUGraphicsPipeline* pipelineObject = nullptr;
	SDL_GPUGraphicsPipeline* pipelineScreen1 = nullptr;

public:
	Gfx(SDL_Window*);

	bool init();
	void cleanup();

	void finish_frame();
};