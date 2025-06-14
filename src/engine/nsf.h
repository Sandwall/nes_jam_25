#pragma once

#include <stdint.h>

namespace nes {
	struct NSF {
		NSF() = default;
		~NSF();

		[[nodiscard]] bool valid() const { return mValid; }
		bool mValid = false;

		bool load(const char* path);
		void cleanup();

		uint8_t version = 0;
		uint8_t numSongs = 0;
		uint8_t startingSong = 0;

		uint16_t loadAddress = 0;
		uint16_t initAddress = 0;
		uint16_t playAddress = 0;

		char songName[32] = { };
		char artist[32] = { };
		char copyrightHolder[32] = { };

		// in 1/1000000th sec ticks
		uint16_t playSpeedNtsc = 0;
		uint16_t playSpeedPal = 0;

		uint8_t bankswitchInitVal[8] = { 0 };

		enum PalNtscBits : uint8_t {
			STATUS_NTSC = 0,
			STATUS_PAL = 1,
			STATUS_DUAL = 2
		} palNtscBits = STATUS_NTSC;

		enum ExtraSoundChipFlag : uint8_t {
			SC_NONE = 0,
			SC_VRC6 = 1 << 0,
			SC_VRC7 = 1 << 1,
			SC_FDS = 1 << 2,
			SC_MMC5 = 1 << 3,
			SC_N163 = 1 << 4,
			SC_S5B = 1 << 5,
			SC_VT02 = 1 << 6,
		} extraSoundChip = SC_NONE;

		enum Nsf2FeatureFlag : uint8_t {
			NSF2_NONE = 0,
			NSF2_IRQ_SUPPORT = 1 << 4,    // if set, this NSF may use the IRQ support features
			NSF2_NO_RET_INIT = 1 << 5,    // if set, the non-returning INIT playback feature will be used
			NSF2_NO_PLAY_SUB = 1 << 6,    // if set, the PLAY subroutine will not be used
			NSF2_APPEND_NSFE = 1 << 7,    // NSFe will not be supported
		} nsf2FeatureFlags = NSF2_NONE;

		uint32_t programLength = 0;
		uint8_t* programData = nullptr;
	};
}
