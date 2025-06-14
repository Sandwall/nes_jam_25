#include "game_context.h"
#include <simdjson.h>

float GameContext::target_sec() const {
	if (targetFps == 0) return 0.0f;
	return 1.0f / static_cast<float>(targetFps);
}

uint64_t GameContext::target_ns() const {
	if (targetFps == 0) return 0;
	return 1000000000LL / targetFps;
}

void* GameContext::jsonParser = nullptr;

// NOTE(sand): I would prefer not to use struct static variables, however seeing as we aren't loading more than one JSON at the same time
// it should be fine to provide GameContext::jsonParser as an single point of access for a single parser
// (simdjson docs say to prefer using a single parser for all parsing to minimize allocations)
void GameContext::init() {
	jsonParser = new simdjson::ondemand::parser();
}

void GameContext::cleanup() {
	delete jsonParser;
	jsonParser = nullptr;
}
