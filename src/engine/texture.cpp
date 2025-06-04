#include "texture.h"

#include <stb_image.h>
#include <stb_rect_pack.h>

#include <stdlib.h>
#include <mems.hpp>

void TextureAtlas::create(int w, int h) {
	width = w;
	height = h;
	nSubtextures = 0;

	data = malloc(width * height * NUM_CHANNELS);
	
}

void TextureAtlas::destroy() {
	width = 0;
	height = 0;
	nSubtextures = 0;

	free(data);
	data = 0;
}

// assumes that mems::init() has been called, as the scratch arena is used
// we'll also impose a 10mb texture size limit
// (assuming uncompressed 4-channel color, this is around a 1024x1024 image)
uint32_t TextureAtlas::add_to_atlas(const char* path) {
	FILE* fp = fopen(path, "rb");
	if (!fp) return INVALID_IDX;

	fseek(fp, 0, SEEK_END);
	long fileSize = ftell(fp);
	constexpr long MAX_FILE_SIZE = 1000 * 1000 * 10;
	if (fileSize > MAX_FILE_SIZE) {
		fclose(fp);
		return INVALID_IDX;
	}

	mems::Arena& scratch = mems::get_scratch();
	mems::ArenaScope scratchScope(scratch);
	
	void* fileData = scratch.push(fileSize);
	fseek(fp, 0, SEEK_SET);
	if (1 != fread(fileData, fileSize, 1, fp)) {
		// fread returns the number of "items" read
		// we're asking fread for 1 item of size fileSize
		// so if we don't get that 1 item, then we need to report an error
		return INVALID_IDX;
	}
	fclose(fp);

	// now we actually load the image
	Subtexture subTex;
	int requestedChannels;
	subTex.data = static_cast<void*>(
		stbi_load_from_memory(
			static_cast<stbi_uc*>(fileData),
			fileSize,
			&subTex.width,
			&subTex.height,
			&requestedChannels, NUM_CHANNELS)
		);

	subtex[nSubtextures++] = subTex;

	// scratchScope goes out of scope so all scratch arena memory from this function is freed
}

void TextureAtlas::pack_atlas() {
	mems::Arena& scratch = mems::get_scratch();
	mems::ArenaScope scratchScope(scratch);

	stbrp_context rpContext;
	stbrp_node* rpNodes = static_cast<stbrp_node*>(scratch.push(sizeof(stbrp_node) * width));
	stbrp_init_target(&rpContext, width, height, rpNodes, width);

	for (int i = 0; i < nSubtextures; i++) {
		rpNodes[i].x = 
	}
	stbrp_rect* rpRects = static_cast<stbrp_rect*>(scratch.push(sizeof(stbrp_rect) * nSubtextures));
	for (int i = 0; i < nSubtextures; i++) {
		// to correlate the rects to their textures
		rpRects->id = i;
	}

	stbrp_pack_rects(&rpContext, rpRects, nSubtextures);
}
