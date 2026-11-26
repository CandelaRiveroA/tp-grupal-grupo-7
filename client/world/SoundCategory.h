#pragma once

namespace world {

// Categorías de audio con volumen propio. Sumar una categoría = un valor
// acá, su fila en la tabla de SettingsLayout, su entrada en
// sdl::AudioMixer y su [[settings.volume]] en ui/settings.toml.
enum class SoundCategory { Sfx, Music };

}  // namespace world
