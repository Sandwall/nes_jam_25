#pragma once

#include <stdint.h>
#include <SDL3/SDL_rect.h>

#include <mems.hpp>

//
// TEXTURES
//

struct SubTexture {
	static constexpr int KEY_LENGTH = 32;

	int x, y;
	int width, height;

	struct SpriteSheet* sheetData;
	char key[KEY_LENGTH];
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

	void pack_atlas();

	// key can be null, in that case it'll just autogenerate a key from the texture idx
	uint32_t add_to_atlas(const char* key, const char* path, const char* jsonPath = nullptr);
	
	// does a linear search, gets the index of the sprite with the key
	// do not use this in the main game loop, it is better to use it to cache a sprite when loading a gameobject
	uint32_t find_sprite(const char* key) const;


	bool isPacked = false;

	int width = -1, height = -1;
	int nSubtextures = -1;

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
	int animIdx;

	// internal
	float timer;
	int currentFrame : (sizeof(int) - 1);
	bool pingpongForward : 1 = true;

	void init(uint32_t sprite);
	void start(int anim, const SpriteSheet& sheet);
	void update(float delta, const SpriteSheet& sheet);
	SDL_FRect current_framef(const SpriteSheet& sheet) const;
	SDL_Rect current_frame(const SpriteSheet& sheet) const;
};