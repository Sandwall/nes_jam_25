#include "nes_apu.h"

namespace nes {
	/* type = duty cycle type
	 * duty = duty cycle index
	 *
	 * this function returns a bit for a specific duty cycle sequence
		*/
	constexpr bool APU::duty_cycle(int type, int duty) {
		constexpr uint8_t PULSE_DUTY_TABLE[4] = {
			0b01000000, // 12.5%
			0b01100000, // 25%
			0b01111000, // 50%
			0b10011111  // 25% negated
		};

		type = type % 4;
		duty = duty % 8;
		return PULSE_DUTY_TABLE[type] >> (7 - duty);
	}

	/* out = buffer of nSamples to write to
	 * NOTE(sand): dmc channel is an integer in [0,127], other channels are an integer
	 * in [0,15], and this gets converted to a range of [0.0, 1.0] when being nonlinear
	 * mixed. However, a transformation is performed at the end, which means the
	 * final output is in the range [-1.0, 1.0]
	 *
	 * Referenced: https://www.nesdev.org/wiki/APU_Mixer
	 */
	void APU::render(float* out, int nSamples) {
		uint8_t pulse1 = 0, pulse2 = 0, triangle = 0, noise = 0, dmc = 0;
		float pulseOut = 0.0f, tndOut = 0.0f;

		for (int i = 0; i < nSamples; i++) {
			// observe registers to generate channel values


			// nonstandard behavior, we are wrapping around each channel
			// in the case that it is too large
			pulse1 %= 16;
			pulse2 %= 16;
			triangle %= 16;
			noise %= 16;
			dmc %= 128;

			// precaution against divide by zero
			if (pulse1 + pulse2 != 0)
				pulseOut = 95.88f / (100.0f + (8128.0f / static_cast<float>(pulse1 + pulse2)));
			else
				pulseOut = 0;

			// calculate the triangle, noise, and dmc components of the tndOut formula
			float tndComponents = (static_cast<float>(triangle) / 8227.0f)
				+ static_cast<float>(noise) / 12241.0f
				+ static_cast<float>(dmc) / 22638.0f;

			// similar precaution against divide by zero
			if (tndComponents != 0.0f)
				tndOut = 159.79f / (100.0f + (1.0f / tndComponents));
			else
				tndOut = 0.0f;

			out[i] = pulseOut + tndOut;
			out[i] = (out[i] - 0.5f) * 2.0f;
		}
	}

	void APU::reset(uint8_t* newReg) {

	}
}