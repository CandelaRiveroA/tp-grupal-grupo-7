#pragma once

#include "core/MusicTrack.h"

namespace core {

// Lo que la capa Qt conoce de la música: pedir una pista o cortarla.
// La implementa sdl::MusicPlayer, así qt/ no depende de SDL.
class MusicControl {
public:
	virtual ~MusicControl() = default;

	virtual void play(MusicTrack track) = 0;

	virtual void stop() = 0;
};

}
