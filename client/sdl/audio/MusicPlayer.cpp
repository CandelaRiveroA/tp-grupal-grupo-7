#include "sdl/audio/MusicPlayer.h"

#include <map>
#include <string>

#include "core/Constants.h"
#include "sdl/audio/AudioConfig.h"

namespace sdl {

MusicPlayer::MusicPlayer() : tracks_{}, current_(nullptr), opened_(false) {
	if (Mix_OpenAudio(AUDIO_FREQUENCY, MIX_DEFAULT_FORMAT, AUDIO_CHANNELS,
			  AUDIO_CHUNK_SIZE) < 0) {
		logger_.warning("MusicPlayer: no se pudo abrir el audio, el lobby queda en silencio");
		logger_.warning(Mix_GetError());
		return;
	}
	opened_ = true;
	Mix_VolumeMusic(MUSIC_VOLUME);

	const std::map<core::MusicTrack, std::string> files = {
		{core::MusicTrack::MenuTheme, MUSIC_MENU_THEME_FILE},
		{core::MusicTrack::CharacterSelect, MUSIC_CHARACTER_SELECT_FILE},
	};
	for (const auto& entry : files) {
		Mix_Music* music = Mix_LoadMUS(core::soundPath(entry.second).c_str());
		if (music == nullptr) {
			logger_.warning("MusicPlayer: una pista no carga y se omite");
			logger_.warning(Mix_GetError());
			continue;
		}
		tracks_[entry.first] = music;
	}
}

MusicPlayer::~MusicPlayer() {
	if (!opened_) {
		return;
	}
	Mix_HaltMusic();
	for (const auto& entry : tracks_) {
		Mix_FreeMusic(entry.second);
	}
	Mix_CloseAudio();
}

void MusicPlayer::play(core::MusicTrack track) {
	const auto found = tracks_.find(track);
	if (!opened_ || found == tracks_.end()) {
		return;
	}
	Mix_Music* music = found->second;

	const bool alreadyPlaying = music == current_ && Mix_PlayingMusic() == 1 &&
				    Mix_FadingMusic() != MIX_FADING_OUT;
	if (alreadyPlaying) {
		return;
	}

	// Mix_FadeInMusic espera en un bucle de 100 ms si hay un fade out en
	// curso; cortarlo antes evita trabar la UI al entrar y salir rápido.
	Mix_HaltMusic();
	if (Mix_FadeInMusic(music, -1, MUSIC_FADE_IN_MS) < 0) {
		logger_.warning("MusicPlayer: no se pudo reproducir la pista");
		logger_.warning(Mix_GetError());
		current_ = nullptr;
		return;
	}
	current_ = music;
}

void MusicPlayer::stop() {
	if (!opened_) {
		return;
	}
	Mix_FadeOutMusic(MUSIC_FADE_OUT_MS);
}

}
