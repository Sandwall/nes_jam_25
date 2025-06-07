#pragma once

struct GameContext;
struct Gfx;

struct GameObject {
	bool active = true;
	int x = 0, y = 0;

	virtual void update(const GameContext& ctx) = 0;
	virtual void render(Gfx& gfx) = 0;
};