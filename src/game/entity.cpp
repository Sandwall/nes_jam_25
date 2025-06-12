#include "entity.h"

#include <tinydef.hpp>

#include "engine/game_context.h"
#include "game/world.h"

bool Entity::is_in_room(const LdtkLevel* level) {
	if (!level) return false;

	const SDL_FRect cbox = get_cboxf();
	const SDL_FRect lbox = level->get_bboxf();
	return SDL_HasRectIntersectionFloat(&cbox, &lbox);
}

SDL_FRect Entity::get_cboxf() {
	return SDL_FRect{ pos.x - origin.x, pos.y - collBoxSize.y, collBoxSize.x, collBoxSize.y };
}