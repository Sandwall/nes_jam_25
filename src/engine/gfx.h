#pragma once

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_render.h>

#define USE_SDL_RENDERER

#define EXPAND_COL(col) col.r, col.g, col.b, col.a

struct TextureAtlas;
struct SubTexture;

struct Gfx {
	static constexpr int nesWidth = 256, nesHeight = 240;
	SDL_FColor clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	SDL_FPoint cameraPos = { 0, 0 };
	uint32_t fontIdx = UINT32_MAX;

	Gfx();

	bool init(SDL_Window*);
	void upload_atlas(const TextureAtlas& atlas);
	void cleanup();

	void queue_point(SDL_FPoint pt, bool useCamera = true, const SDL_FColor& color = { 1.0f, 1.0f, 1.0f, 1.0f });
	void queue_rect(SDL_FRect dest, bool useCamera = true, const SDL_FColor& color = { 1.0f, 1.0f, 1.0f, 1.0f });
	// always draws text @ font height 8
	void queue_text(int x, int y, const char* text, const SDL_FColor& color = { 1.0f, 1.0f, 1.0f, 1.0f });

	void queue_sprite(int x, int y, uint32_t spriteIdx, const SDL_Rect& src, bool useCamera = true, const SDL_FColor& color = { 1.0f, 1.0f, 1.0f, 1.0f });
	void queue_sprite(int x, int y, const SubTexture& subTex, const SDL_Rect& src, bool useCamera = true, const SDL_FColor& color = { 1.0f, 1.0f, 1.0f, 1.0f });


	// these are commented because the compiler has trouble with overloads if you omit the color argument

	//void queue_rect(const SDL_FRect& dest, const SDL_FRect& src, const SDL_Color& color = { 255, 255, 255, 255 });
	//void queue_rect(const SDL_FRect& dest, const SDL_Color& color = { 255, 255, 255, 255 });

	void begin_frame();
	void finish_frame();

	// utility functions
	static SDL_FColor hsv_to_col(float h, float s, float v, float a);
	SDL_FPoint world_to_screen(SDL_FPoint pt) const;
	SDL_FPoint screen_to_world(SDL_FPoint pt) const;
	SDL_Rect cam_bbox() const;
	SDL_FRect cam_bboxf() const;

private:
	const TextureAtlas* spriteAtlas = nullptr;
	SDL_Window* windowPtr = nullptr;

#ifndef USE_SDL_RENDERER
	//
	// SDL_GPU
	//

	SDL_GPUDevice* device = nullptr;

	SDL_GPUTexture* textureAtlas = nullptr;
	SDL_GPUTexture* textureScreen1 = nullptr;
	SDL_GPUTexture* textureDepth = nullptr;

	SDL_GPUShader* vertShaderObject = nullptr;
	SDL_GPUShader* fragShaderObject = nullptr;
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
};
