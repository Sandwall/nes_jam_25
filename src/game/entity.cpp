#include "entity.h"

#include "world.h"

bool Entity::is_in_room(const LdtkLevel* level) {
	if (!level) return false;

	const SDL_FRect cbox = get_cboxf();
	const SDL_FRect lbox = level->get_bboxf();
	return SDL_HasRectIntersectionFloat(&cbox, &lbox);
}

SDL_FRect Entity::get_cboxf() {
	return SDL_FRect{ pos.x - origin.x, pos.y - origin.y, collBoxSize.x, collBoxSize.y };
}
