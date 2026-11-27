#include "sdl/audio/AudioMixer.h"

#include <SDL2/SDL_mixer.h>

#include "world/WorldConstants.h"

namespace sdl {

AudioMixer::AudioMixer():
        appliers_{
                // -1 = todos los canales (por donde suenan los efectos).
                {SoundCategory::Sfx, [](const int volume) { Mix_Volume(-1, volume); }},
                {SoundCategory::Music, [](const int volume) { Mix_VolumeMusic(volume); }},
        } {}

void AudioMixer::apply(const std::map<SoundCategory, int>& volumes) {
    for (const auto& [category, percent]: volumes) {
        appliers_.at(category)(sdlVolume(percent));
    }
}

int AudioMixer::sdlVolume(const int percent) const {
    return percent * MIX_MAX_VOLUME / world::kMaxVolumePercent;
}

}  // namespace sdl
