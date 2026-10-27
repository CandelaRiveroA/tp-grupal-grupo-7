#pragma once

#include <SDL2/SDL_mixer.h>

#include <array>

#include "sdl/audio/AudioCatalog.h"

namespace sdl {

class AudioPlayer {
public:
	AudioPlayer();
	~AudioPlayer();

	AudioPlayer(const AudioPlayer&) = delete;
	AudioPlayer& operator=(const AudioPlayer&) = delete;

	void play(SoundId id);

private:
	std::array<Mix_Chunk*, kSoundCount> chunks_;
};

}
