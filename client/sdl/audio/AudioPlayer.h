#pragma once

#include <SDL2/SDL_mixer.h>

#include <array>
#include <utility>
#include <vector>

#include "game/SoundCue.h"
#include "sdl/audio/AudioCatalog.h"

namespace sdl {

class AudioPlayer {
public:
	AudioPlayer();
	~AudioPlayer();

	AudioPlayer(const AudioPlayer&) = delete;
	AudioPlayer& operator=(const AudioPlayer&) = delete;

	void play(SoundId id);

	void playCue(game::CueKind cue);

private:
	std::array<Mix_Chunk*, kSoundCount> chunks_;

	std::vector<std::pair<game::CueKind, SoundId>> cueSounds_;
};

}
