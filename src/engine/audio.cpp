#include "audio.h"
#include "nsf.h"
#include "nes_apu.h"

#include <SDL3/SDL_audio.h>

//nes::NSF songs[3];
//nes::APU apu;

SDL_AudioStream* mainStream;

static constexpr int BLOCK_SIZE = 512;

float audioBuf[BLOCK_SIZE];
void fill_buffer(int n) {
	memset(audioBuf, 0, sizeof(float) * n);
}

void audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount) {
	while (additional_amount > BLOCK_SIZE) {
		fill_buffer(BLOCK_SIZE);

		// have to fill audioBuf with additional_amount values and push that with the below function
		SDL_PutAudioStreamData(stream, audioBuf, BLOCK_SIZE);
		additional_amount -= BLOCK_SIZE;
	}

	if (additional_amount > 0) {
		fill_buffer(additional_amount);
		SDL_PutAudioStreamData(stream, audioBuf, additional_amount);
	}
}


void audio_init() {
	//songs[0].load("./res/test.nsf");
	const SDL_AudioSpec aSpec = {
		.format = SDL_AUDIO_F32,
		.channels = 1,
		.freq = nes::PLAYBACK_SAMPLE_RATE,
	};

	mainStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &aSpec, audio_callback, nullptr);
	SDL_ResumeAudioStreamDevice(mainStream);
}

void audio_tick() {

}

void audio_close() {
	//songs[0].cleanup();
	SDL_DestroyAudioStream(mainStream);
}