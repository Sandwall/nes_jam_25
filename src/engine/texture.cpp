#include "texture.h"

#include <stb_image.h>
#include <stb_rect_pack.h>

#include <stdlib.h>
#include <string.h>
#include <mems.hpp>

void TextureAtlas::create(int w, int h) {
	width = w;
	height = h;
	nSubtextures = 0;

	data = malloc(width * height * NUM_CHANNELS);
	isPacked = false;
}

void TextureAtlas::destroy() {
	width = 0;
	height = 0;
	nSubtextures = 0;

	free(data);
	data = 0;
	isPacked = false;
}

// assumes that mems::init() has been called, as the scratch arena is used
// we'll also impose a 10mb texture size limit
// (assuming uncompressed 4-channel color, this is around a 1024x1024 image)
uint32_t TextureAtlas::add_to_atlas(const char* path) {
	if (isPacked)	// cannot add to an already packed atlas
		return UINT32_MAX;

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
	SubTexture subTex;
	int requestedChannels;
	subTex.data = static_cast<void*>(
		stbi_load_from_memory(
			static_cast<stbi_uc*>(fileData),
			fileSize,
			&subTex.width,
			&subTex.height,
			&requestedChannels, NUM_CHANNELS)
		);

	subTextures[nSubtextures++] = subTex;

	// scratchScope goes out of scope so all scratch arena memory from this function is freed
}

// this function packs the rects into the atlas and frees the subtextures on the CPU-side
void TextureAtlas::pack_atlas() {
	if (isPacked)
		return;

	mems::Arena& scratch = mems::get_scratch();
	mems::ArenaScope scratchScope(scratch);

	stbrp_context rpContext;
	stbrp_node* rpNodes = static_cast<stbrp_node*>(scratch.push(sizeof(stbrp_node) * width));
	stbrp_init_target(&rpContext, width, height, rpNodes, width);

	stbrp_rect* rpRects = static_cast<stbrp_rect*>(scratch.push(sizeof(stbrp_rect) * nSubtextures));
	for (int i = 0; i < nSubtextures; i++) {
		// to correlate the rects to their textures
		rpRects->id = i;

		rpRects->x = 0;
		rpRects->y = 0;
		rpRects->w = subTextures[i].width;
		rpRects->h = subTextures[i].height;
	}

	stbrp_pack_rects(&rpContext, rpRects, nSubtextures);

	int rectsNotPacked = 0;
	for (int i = 0; i < nSubtextures; i++) {
		const stbrp_rect& rect = rpRects[i];
		SubTexture& cSubtex = subTextures[rect.id];

		if (0 != rect.was_packed) {
			cSubtex.x = rect.x;
			cSubtex.y = rect.y;
			cSubtex.width = rect.w;
			cSubtex.height = rect.h;

			_move_subtex_to_atlas(rect.id, rect.x, rect.y);
		} else {
			rectsNotPacked++;
		}

	}

	isPacked = true;
}

// copies subtex at idx to the main atlas data, at the destination rectangle with the top left position (x,y)
void TextureAtlas::_move_subtex_to_atlas(int idx, int x, int y) {
	SubTexture& subtex = subTextures[idx];

	// each subtexture was loaded with 4 channels, which the same way as
	// the actual texture data will be interpreted by the gpu,
	// so we can just memcpy each horizontal line of pixels onto the atlas
	for (int i = 0; i < subtex.height; i++) {
		uint32_t* src = static_cast<uint32_t*>(subtex.data) + (subtex.width * i);
		uint32_t* dst = static_cast<uint32_t*>(data) + x + ((i+y) * width);
		memcpy(dst, src, NUM_CHANNELS * subtex.width);
	}

	stbi_image_free(subtex.data);
	subtex.data = nullptr;
}