#pragma once

// Parámetros del dispositivo de audio. Los usan AudioPlayer (juego) y
// MusicPlayer (lobby)
#define AUDIO_FREQUENCY 44100
#define AUDIO_CHANNELS 2
#define AUDIO_CHUNK_SIZE 2048

// Músicas de fondo (relativas a core::soundPath).
#define MUSIC_MENU_THEME_FILE "music/menu_theme.ogg"

// Fade in al empezar una pista y fade out al cortarla.
#define MUSIC_FADE_IN_MS 1000
#define MUSIC_FADE_OUT_MS 400


#define MUSIC_VOLUME (MIX_MAX_VOLUME / 2)
