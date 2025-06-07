#define _CRT_SECURE_NO_WARNINGS

#include "image_asset.h"

#include <simdjson.h>

#include <stb_image.h>
#include <stb_rect_pack.h>
#include <mems.hpp>

#include <stdlib.h>
#include <string.h>

void TextureAtlas::create(int w, int h) {
	width = w;
	height = h;
	nSubtextures = 0;
	memset(subTextures, 0, sizeof(SubTexture) * MAX_SUBTEXTURES);
	memset(&arena, 0, sizeof(mems::Arena));

	arena.alloc();
	data = arena.push(width * height * NUM_CHANNELS);

	isPacked = false;
}

void TextureAtlas::destroy() {
	width = 0;
	height = 0;
	nSubtextures = 0;
	memset(subTextures, 0, sizeof(SubTexture) * MAX_SUBTEXTURES);

	arena.dealloc();
	data = nullptr;
	isPacked = false;
}

// assumes that mems::init() has been called, as the scratch arena is used
// we'll also impose a 10mb texture size limit
// (assuming uncompressed 4-channel color, this is around a 1024x1024 image)
uint32_t TextureAtlas::add_to_atlas(const char* imagePath, const char* jsonPath) {
	if (isPacked)	// cannot add to an already packed atlas
		return UINT32_MAX;

	FILE* fp = fopen(imagePath, "rb");
	if (!fp) {
		fprintf(stderr, "Could not add %s to TextureAtlas!\n", imagePath);
		return INVALID_IDX;
	}

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
	SubTexture subTex = {};
	int requestedChannels;
	subTex.data = static_cast<void*>(
		stbi_load_from_memory(
			static_cast<stbi_uc*>(fileData),
			fileSize,
			&subTex.width,
			&subTex.height,
			&requestedChannels, NUM_CHANNELS)
		);

	subTextures[nSubtextures] = subTex;

	if (jsonPath)
		subTex.sheetData = SpriteSheet::load(jsonPath, arena);
	else
		subTex.sheetData = nullptr;

	return nSubtextures++;
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
		rpRects[i].id = i;
		rpRects[i].w = subTextures[i].width;
		rpRects[i].h = subTextures[i].height;
	}

	if (1 != stbrp_pack_rects(&rpContext, rpRects, nSubtextures)) {
		fprintf(stderr, "stb_rect_pack couldn't pack all textures!\n");
	}

	int rectsNotPacked = 0;
	for (int i = 0; i < nSubtextures; i++) {
		const stbrp_rect& rect = rpRects[i];
		SubTexture& cSubtex = subTextures[rect.id];

		if (0 != rect.was_packed) {
			cSubtex.x = rect.x;
			cSubtex.y = rect.y;
			cSubtex.width = rect.w;
			cSubtex.height = rect.h;

			_move_subtex_to_atlas(rect.id);
		} else {
			rectsNotPacked++;
		}
	}

	if (rectsNotPacked != 0) {
		fprintf(stderr, "%d textures not packed!", rectsNotPacked);
	}

	isPacked = true;
}

// copies subtex at idx to the main atlas data, at the destination rectangle with the top left position (x,y)
void TextureAtlas::_move_subtex_to_atlas(int idx) {
	SubTexture& subtex = subTextures[idx];

	// each subtexture was loaded with 4 channels, which the same way as
	// the actual texture data will be interpreted by the gpu,
	// so we can just memcpy each horizontal line of pixels onto the atlas

	//for (int iy = 0; iy < subtex.height; iy++) {
	//	for (int ix = 0; ix < subtex.width; ix++) {
	//		uint32_t* src = static_cast<uint32_t*>(subtex.data) + (iy * subtex.width) + ix;
	//		uint32_t* dst = static_cast<uint32_t*>(data) + ((subtex.y + iy) * width) + subtex.x + ix;
	//		*dst = *src;
	//	}
	//}

	for (int i = 0; i < subtex.height; i++) {
		uint32_t* src = static_cast<uint32_t*>(subtex.data) + (subtex.width * i);
		uint32_t* dst = static_cast<uint32_t*>(data) + ((subtex.y + i) * width) + subtex.x;
		memcpy(dst, src, NUM_CHANNELS * subtex.width);
	}

	stbi_image_free(subtex.data);
	subtex.data = nullptr;
}



using namespace simdjson;

#define JSON_CHECK_ERR(value) \
	if (auto err = value.error() != false) { \
		fprintf(stderr, "simdjson error %d: was unable to load spritesheet metadata from %s\n", err, jsonPath); \
		return false; \
	}

// This function loads a spritesheet from a json filepath, into arena-allocated storage
SpriteSheet* SpriteSheet::load(const char* jsonPath, mems::Arena& arena) {
	SpriteSheet* sheet = static_cast<SpriteSheet*>(arena.push_zero(sizeof(SpriteSheet)));

	ondemand::parser parser;
	padded_string json = padded_string::load(jsonPath);
	ondemand::document sheetDoc = parser.iterate(jsonPath);

	// load source frame rects
	auto framesVal = sheetDoc["frames"].get_array();
	sheet->nFrames = framesVal.count_elements();
	sheet->frames = static_cast<AnimationFrame*>(arena.push_zero(sizeof(AnimationFrame*) * sheet->nFrames));

	for (int i = 0; i < sheet->nFrames; i++) {
		auto frameData = framesVal.at(i);
		auto frameRect = frameData["frame"];
		sheet->frames[i].source.x = frameRect["x"];
		sheet->frames[i].source.y = frameRect["y"];
		sheet->frames[i].source.w = frameRect["w"];
		sheet->frames[i].source.h = frameRect["h"];
		sheet->frames[i].duration = static_cast<float>(frameData["duration"]) / 1000.0f;
	}

	auto frameTags = sheetDoc["meta"]["frameTags"];
	sheet->nAnimations = frameTags.count_elements();
	sheet->anims = static_cast<AnimationMeta*>(arena.push_zero(sizeof(AnimationMeta*) * sheet->nAnimations));

	for (int i = 0; i < sheet->nAnimations; i++) {
		auto frameTag = frameTags.at(i);
		sheet->anims[i].startFrame = frameTag["from"];
		sheet->anims[i].endFrame = frameTag["to"];

		const auto& dirStr = frameTag["direction"].get_string().value();
		if ("forward" == dirStr) {
			sheet->anims[i].type = AnimationMeta::FORWARD;
		} else if ("pingpong" == dirStr) {
			sheet->anims[i].type = AnimationMeta::PINGPONG;
		} else if ("backward" == dirStr) {
			sheet->anims[i].type = AnimationMeta::BACKWARD;
		} else {
			sheet->anims[i].type = AnimationMeta::FORWARD;
		}
	}

	return sheet;
}
