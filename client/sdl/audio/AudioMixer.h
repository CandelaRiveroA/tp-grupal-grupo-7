#pragma once

#include <functional>
#include <map>

#include "sdl/audio/AudioCatalog.h"

namespace sdl {

// Único lugar que traduce el volumen elegido por el jugador (% por
// categoría) al volumen de SDL_mixer. Cada categoría sabe cómo aplicarse
// (efectos: los canales; música: el stream de música). Sumar una categoría
// es una entrada en esa tabla. Requiere el audio abierto (AudioPlayer).
class AudioMixer {
public:
    AudioMixer();

    // Aplica el volumen (%) de cada categoría recibida; las que no vienen
    // quedan como están.
    void apply(const std::map<SoundCategory, int>& volumes);

private:
    // Porcentaje (0-100) -> volumen de SDL_mixer (0-MIX_MAX_VOLUME).
    int sdlVolume(int percent) const;

    std::map<SoundCategory, std::function<void(int)>> appliers_;
};

}  // namespace sdl
