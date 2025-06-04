#pragma once

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_render.h>

#include <vector>

#define USE_SDL_RENDERER

#define EXPAND_COL(col) col.r, col.g, col.b, col.a

class Gfx {
	SDL_Window* windowPtr = nullptr;

#ifndef USE_SDL_RENDERER
	//
	// SDL_GPU
	//

	SDL_GPUDevice* device = nullptr;

	SDL_GPUTexture* textureAtlas = nullptr;
	SDL_GPUTexture* textureScreen1 = nullptr;
	SDL_GPUTexture* textureDepth = nullptr;

	SDL_GPUShader* vertShaderObject  = nullptr;
	SDL_GPUShader* fragShaderObject  = nullptr;
	SDL_GPUShader* vertShaderScreen1 = nullptr;
	SDL_GPUShader* fragShaderScreen1 = nullptr;

	SDL_GPUBuffer* bufferObject = nullptr;
	SDL_GPUBuffer* bufferScreen = nullptr;

	SDL_GPUSampler* samplerNearest = nullptr;

	SDL_GPUGraphicsPipeline* pipelineObject = nullptr;
	SDL_GPUGraphicsPipeline* pipelineScreen1 = nullptr;

	SDL_GPUColorTargetInfo ctiObject;
	SDL_GPUDepthStencilTargetInfo dstiObject;
	SDL_GPUColorTargetInfo ctiScreen1;
#else
	// SDL_Renderer
	SDL_Renderer* renderer = nullptr;
	SDL_Texture* textureAtlas = nullptr;
	SDL_Texture* textureScreen1 = nullptr;

#endif

public:
	static constexpr int nesWidth = 256, nesHeight = 240;

	Gfx(SDL_Window*);

	bool init();
	void cleanup();

	SDL_FColor clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	SDL_Rect camera = { 0, 0, nesWidth, nesHeight };
	
	void queue_rect(SDL_FRect dest, const SDL_FRect& src, const SDL_FColor& color = { 1.0f, 1.0f, 1.0f, 1.0f });
	void queue_rect(SDL_FRect dest, const SDL_FColor& color = { 1.0f, 1.0f, 1.0f, 1.0f });

	// these are commented because the compiler has trouble with overloads if you omit the color argument

	//void queue_rect(const SDL_FRect& dest, const SDL_FRect& src, const SDL_Color& color = { 255, 255, 255, 255 });
	//void queue_rect(const SDL_FRect& dest, const SDL_Color& color = { 255, 255, 255, 255 });

	void begin_frame();
	void finish_frame();

	// utility functions
	static SDL_FColor hsv_to_col(float h, float s, float v, float a);
};
