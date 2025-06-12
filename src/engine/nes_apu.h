#pragma once

#include <stdint.h>


namespace nes {
	static constexpr int PLAYBACK_SAMPLE_RATE = 44100;
	
	struct Cpu6502 {
		uint16_t pc;
		uint8_t sp, a, x, y, status;

		// only allocate enough memory just for the NES APU's usage
		uint8_t memory[0x8000];
	};

	// Low-Level implementation of the NES APU
	//
	// Set the variables using set_*, and these change the state of the APU
	// stored in the registers (reg[24]). From there, an arbitrary number
	// of samples of audio with this state can be written to a
	// mono 32-bit floating point buffer
	struct APU {
		static constexpr bool duty_cycle(int type, int duty);

		void render(float* out, int nSamples);
		void reset(uint8_t* newReg);

		Cpu6502 cpu;

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
		} p1 = { &cpu.memory[0x4000] }, p2 = { &cpu.memory[0x4004] };

		struct Triangle {
			bool counter_halt() const { return (reg[0] >> 7) % 1; }
			uint8_t counter_load() const { return reg[0] & 127; }

			uint8_t* reg;
		} tri = { &cpu.memory[0x4008] };

		struct Noise {
			uint8_t* reg;
		} noise = { &cpu.memory[0x400C] };

		struct DMC {
			uint8_t* reg;
		} dmc = { &cpu.memory[0x4010] };

		uint8_t* status = &cpu.memory[0x4015];
		uint8_t* frameCounter = &cpu.memory[0x4017];

		// pulse 1 and 2 are channels 0 and 1
		// triangle is channel 2
		// noise is channel 3
		// DMC is channel 4
		void set_channel(uint8_t channel, bool on = true) {
		}

		bool is_channel_on(uint8_t channel) {
			return (*status >> channel) % 1;
		}

	};
}