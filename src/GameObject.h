#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#define INPUT_COLOR(currentColor) currentColor.r, currentColor.g, currentColor.b, currentColor.a
#define FILE_NAME(file) file

#include "SDL3_image/SDL_image.h"
#include "SDL3/SDL.h"
#include "stb_image.h"

struct GameObject
{
public:
	GameObject();

	void initialize_game_object(const char* fileName_, SDL_Window* window_);
	void clean_game_object();

	void render_game_object(SDL_Window* window_, SDL_FRect& dest_, const SDL_FColor& color_ = {1.0f, 1.0f, 1.0f, 1.0f});
	bool collision_with_game_object(SDL_FRect& rect1_, SDL_FRect& rect2_);

private:
	SDL_Surface* surface;
	SDL_Texture* texture;

	SDL_FRect src;
	SDL_FRect dest;
};

#endif GAMEOBJECT_H