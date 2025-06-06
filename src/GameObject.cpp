#include "GameObject.h"
#include <cstdio>

GameObject::GameObject() : src(), dest(), surface(NULL), texture(NULL)
{
}

void GameObject::initialize_game_object(const char* fileName_, SDL_Window* window_)
{
	// Load the texture file
	surface = IMG_Load(FILE_NAME(fileName_));

	// If the texture file isn't valid, print can't load file message
	if (surface == NULL) printf("Couldn't load %s\n", fileName_);

	// Use the texture to render from the SDL's surface
	texture = SDL_CreateTextureFromSurface(SDL_GetRenderer(window_), surface);

	// Destroy surface after it's set
	SDL_DestroySurface(surface);

	// Get the texture size using the reference operator of source rectangle's width and height
	SDL_GetTextureSize(texture, &src.w, &src.h);
}

void GameObject::clean_game_object()
{
	SDL_DestroyTexture(texture);
}

void GameObject::render_game_object(SDL_Window* window_, SDL_FRect& dest_, const SDL_FColor& color_)
{
	// Draw a section of the image
	src = { 0, 0, dest_.w, dest_.h };

	// Draw texture somewhere on the window
	dest = { dest_.x, dest_.y, dest_.w, dest_.h };

	// Render the texture to go along with it
	SDL_RenderTexture(SDL_GetRenderer(window_), texture, &src, &dest);

	// Draw rectangle
	//SDL_SetRenderDrawColorFloat(SDL_GetRenderer(window_), INPUT_COLOR(color_));
	//SDL_RenderFillRect(SDL_GetRenderer(window_), &dest_);
}

bool GameObject::collision_with_game_object(SDL_FRect& rect1_, SDL_FRect& rect2_)
{
	if (rect1_.x >= rect2_.x && rect1_.x <= rect2_.x + rect2_.w &&
		rect1_.y >= rect2_.y && rect1_.y <= rect2_.y + rect2_.h)
	{
		printf("Collision\n");
	}

	return rect1_.x >= rect2_.x && rect1_.x <= rect2_.x + rect2_.w &&
		rect1_.y >= rect2_.y && rect1_.y <= rect2_.y + rect2_.h;
}
