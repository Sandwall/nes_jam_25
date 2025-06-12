#define _CRT_SECURE_NO_WARNINGS

#include "nsf.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

namespace nes {
	NSF::~NSF() { cleanup(); }

	void NSF::cleanup() {
		if (programData) {
			delete[] programData;
			programData = nullptr;
			mValid = false;
		}
	}

	uint16_t make_word(uint8_t lo, uint8_t hi) {
		return static_cast<uint16_t>(lo) | (static_cast<uint16_t>(hi) << 8);
	}

	uint32_t make_3byte(uint8_t b0, uint8_t b1, uint8_t b2) {
		return static_cast<uint32_t>(b0) | (static_cast<uint32_t>(b1) << 8) | (static_cast<uint32_t>(b2) << 16);
	}

	bool NSF::load(const char* path) {
		mValid = false;

		FILE* fp = fopen(path, "rb");
		if (!fp) return false;

		fseek(fp, 0, SEEK_END);
		size_t fileSize = ftell(fp);
		if (fileSize < 0x80) {
			// file doesn't even have the size of a full header in it
			fclose(fp);
			return false;
		}
		fseek(fp, 0, SEEK_SET);

		char* buffer = new char[fileSize];
		if (1 != fread(buffer, fileSize, 1, fp)) {
			fclose(fp);
			delete[] buffer;
			return false;
		}
		fclose(fp);

		char* bufPtr = buffer;

		// validate header
		constexpr char C_HEADER[5] = { 'N', 'E', 'S', 'M', 0x1A };
		if (0 != memcmp(bufPtr, C_HEADER, 5))
			goto nsf_load_fail_and_free;

		bufPtr += 5;
		version = static_cast<uint8_t>(*(bufPtr++));
		numSongs = static_cast<uint8_t>(*(bufPtr++));
		startingSong = static_cast<uint8_t>(*(bufPtr++));

		loadAddress = make_word(bufPtr[0], bufPtr[1]);
		bufPtr += 2;
		initAddress = make_word(bufPtr[0], bufPtr[1]);
		bufPtr += 2;
		playAddress = make_word(bufPtr[0], bufPtr[1]);
		bufPtr += 2;

		strncpy(songName, bufPtr, 32);
		bufPtr += 32;
		strncpy(artist, bufPtr, 32);
		bufPtr += 32;
		strncpy(copyrightHolder, bufPtr, 32);
		bufPtr += 32;

		playSpeedNtsc = make_word(bufPtr[0], bufPtr[1]);
		bufPtr += 2;
		playSpeedPal = make_word(bufPtr[0], bufPtr[1]);
		bufPtr += 2;

		memcpy(bankswitchInitVal, bufPtr, 8);
		bufPtr += 8;

		palNtscBits = static_cast<PalNtscBits>(*(bufPtr++));
		extraSoundChip = static_cast<ExtraSoundChipFlag>(*(bufPtr++));
		nsf2FeatureFlags = static_cast<Nsf2FeatureFlag>(*(bufPtr++));

		programLength = make_3byte(bufPtr[0], bufPtr[1], bufPtr[2]);
		bufPtr += 3;

		if (programLength == 0) {
			programLength = fileSize - 0x80;
			programData = new uint8_t[programLength];
			memcpy(programData, buffer + 0x80, programLength);
		}
		else {
			programData = new uint8_t[programLength];
			memcpy(programData, buffer + 0x80, programLength);
		}

		delete[] buffer;
		return true;

nsf_load_fail_and_free:
		delete[] buffer;
		return false;
	}
}