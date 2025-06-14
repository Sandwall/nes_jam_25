#pragma once

#include <stdint.h>

namespace nes {
	static constexpr int PLAYBACK_SAMPLE_RATE = 44100;
	
	// Minimal implementation of 6502 CPU with regards to NES APU and NSF playing
	class APU {
		struct Instruction {
			uint8_t opcode;
			uint8_t data1;
			uint8_t data2;
			uint8_t length;
		};
	public:
		static constexpr int TOTAL_MEMORY = 0x10000;

		enum StatusFlag : uint8_t {
			SF_CARRY             = 1 << 0,
			SF_ZERO              = 1 << 1,
			SF_INTERRUPT_DISABLE = 1 << 2,
			SF_DECIMAL           = 1 << 3,
			SF_BREAK             = 1 << 4,
			SF_OVERFLOW          = 1 << 5,
			SF_NEGATIVE          = 1 << 6,
		};

		uint16_t programCtr;
		uint8_t stackPtr, aReg, xReg, yReg, statusReg;
		uint8_t memory[TOTAL_MEMORY];

		Instruction _fetch();
		void jump_subroutine(uint16_t address);
		void step();

		bool load_song(const struct NSF& nsf);
		void start_song(const struct NSF& nsf);
		
		static constexpr bool duty_cycle(int type, int duty);

		// renders to a mono 32-bit floating point buffer at a 44100 sample rate
		void render(float* out, int nSamples);
		void reset(uint8_t* newReg);

		// Registers are mapped to $4000-400F, $4015, $4017
		struct Pulse {
			uint8_t duty() const { return (reg[0] >> 6) % 4; }
			bool counter_halt() const { return (reg[0] >> 5) % 1; }
			bool const_vol() const { return (reg[0] >> 4) % 1; }
			uint8_t env_vol() const { return reg[0] & 15; }
			bool sweep_on() const { return (reg[1] >> 7) % 1; }
			uint8_t sweep_period() const { return (reg[1] >> 4) % 8; }
			bool sweep_negate() const { return (reg[1] >> 3) % 1; }
			bool sweep_shift() const { return reg[1] % 8; }
			uint8_t counter_load() const { return (reg[3] >> 3) & 31; }
			uint16_t timer() const {
				return static_cast<uint16_t>(reg[2]) |
					   (static_cast<uint16_t>(reg[3] % 8) << 8);
			}

			uint8_t* reg;
		} p1 = { &memory[0x4000] }, p2 = { &memory[0x4004] };

		struct Triangle {
			bool counter_halt() const { return (reg[0] >> 7) % 1; }
			uint8_t counter_load() const { return reg[0] & 127; }

			uint8_t* reg;
		} tri = { &memory[0x4008] };

		struct Noise {
			uint8_t* reg;
		} noise = { &memory[0x400C] };

		struct DMC {
			uint8_t* reg;
		} dmc = { &memory[0x4010] };

		
		uint8_t* apuStatus = &memory[0x4015];
		uint8_t* frameCounter = &memory[0x4017];

		// pulse 1 and 2 are channels 0 and 1
		// triangle is channel 2
		// noise is channel 3
		// DMC is channel 4
		void set_channel(uint8_t channel, bool on = true) {
		}

		bool is_channel_on(uint8_t channel) {
			return (*apuStatus >> channel) % 1;
		}

	};
}