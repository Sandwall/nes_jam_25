#define _CRT_SECURE_NO_WARNINGS

#define STB_IMAGE_IMPLEMENTATION
#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_image.h>
#include <stb_rect_pack.h>

#include <simdjson.cpp>

#define MEMS_IMPLEMENTATION
#include <mems.hpp>

/* libs.cpp
 * Our project will make use of stb_image to load PNG files as well as stb_rect_pack to pack each image into a single texture atlas.
 * These are single-header libraries, which means that they contain two sections inside of them, the declarations and implementation.
 *
 * This is similar to how most C++ projects have a declaration file (*.h) and implementation file (*.cpp), so we're just leaving the
 * implementations of stb_image.h and stb_rect_pack.h in this file. It's better to do this instead of place the implementation in something
 * like engine/gfx.cpp as we're more likely to touch and recompile this file less than engine/gfx.cpp, and that'll waste less time
 * on the incremental linking side.
 */