#pragma once


struct Level {
	// allocates and deallocates memory for the level
	void init();
	void cleanup();

	// loads level in from file
	void load(const char* path);
};