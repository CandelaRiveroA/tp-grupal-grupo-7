#pragma once

#include <SDL2/SDL_mixer.h>

#include <map>

#include "core/MusicControl.h"
#include "core/MusicTrack.h"
#include "sdl/log/Logger.h"

namespace sdl {

class MusicPlayer : public core::MusicControl {
public:
	MusicPlayer();
	~MusicPlayer() override;

	MusicPlayer(const MusicPlayer&) = delete;
	MusicPlayer& operator=(const MusicPlayer&) = delete;

	void play(core::MusicTrack track) override;
	void stop() override;

private:
	// Las pistas se liberan recién en el destructor
	std::map<core::MusicTrack, Mix_Music*> tracks_;
	Mix_Music* current_;
	bool opened_;
	Logger logger_;
};

}
