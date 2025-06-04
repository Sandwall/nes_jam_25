#pragma once

#include <stdint.h>
#include <stb_rect_pack.h>

struct Subtexture {
	int x, y;
	int width, height;
	void* data;
};

struct TextureAtlas {
	static constexpr uint32_t INVALID_IDX = UINT32_MAX;
	static constexpr int NUM_CHANNELS = 4;    // RGBA, this is hardcoded for now
	static constexpr int MAX_SUBTEXTURES = 128;
	
	void create(int w = 4096, int h = 4096);
	void destroy();

	uint32_t add_to_atlas(const char* path);
	void pack_atlas();
	// notice how there is no remove_from_atlas - we don't need one :)

	int width, height;
	int nSubtextures;
	void* data = nullptr;

	Subtexture subtex[MAX_SUBTEXTURES];
};