#pragma once

namespace core {

// Vocabulario de las músicas de fondo. Lo comparten la capa Qt (que pide
// una pista) y la capa SDL (que sabe cómo reproducirla); agregar una pista
// es sumar un valor acá y su archivo en sdl::MusicPlayer.
enum class MusicTrack {
	MenuTheme,
};

}
