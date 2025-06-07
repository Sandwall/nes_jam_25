#pragma once

#include <stdint.h>
#include <SDL3/SDL_rect.h>

#include <mems.hpp>

struct SpriteSheet;

//
// TEXTURES
//

struct SubTexture {
	int x, y;
	int width, height;

	SpriteSheet* sheetData;
private:
	// Only TextureAtlas gets to manage the CPU side texture data
	friend struct TextureAtlas;
	void* data;
};

struct TextureAtlas {
	static constexpr uint32_t INVALID_IDX = UINT32_MAX;
	static constexpr int NUM_CHANNELS = 4;    // RGBA, this is hardcoded for now
	static constexpr int MAX_SUBTEXTURES = 128;
	
	void create(int w, int h);
	void destroy();

	uint32_t add_to_atlas(const char* path, const char* jsonPath = nullptr);
	void pack_atlas();
	// notice how there is no remove_from_atlas - we don't need one :)

	bool isPacked;

	int width, height;
	int nSubtextures;

	void* data = nullptr;
	SubTexture subTextures[MAX_SUBTEXTURES];
private:
	void _move_subtex_to_atlas(int idx);
	mems::Arena arena;
};

//
// ANIMATIONS
//

struct AnimationFrame {
	SDL_Rect source;

	// aseprite json keeps these in milliseconds
	// but our code will convert these and interpret them as seconds
	float duration;
};

struct AnimationMeta {
	enum Type {
		FORWARD,
		BACKWARD,
		PINGPONG
	} type;

	int startFrame;
	int endFrame;
};

// AnimationFrame and AnimationMeta are managed by this struct
struct SpriteSheet {
	int nFrames;
	int nAnimations;
	AnimationFrame* frames;
	AnimationMeta* anims;

	static SpriteSheet* load(const char* jsonPath, mems::Arena& arena);
};

struct SpriteAnimator {
	// this is the offset in the TextureAtlas for both SubTexture and SpriteSheet
	uint32_t spriteIdx;

	// internal
	float timer;
	
	// output
	SDL_Rect current;

	void start(bool resetAnim = true);
	void update(float delta);
};