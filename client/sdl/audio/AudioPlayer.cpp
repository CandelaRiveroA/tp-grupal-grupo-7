#include "AudioPlayer.h"

#include <SDL2pp/SDL2pp.hh>

#include "core/Constants.h"

namespace sdl {

namespace {

constexpr int kFrequency = 44100;
constexpr int kChannels = 2;
constexpr int kChunkSize = 2048;

}

AudioPlayer::AudioPlayer() : chunks_{} {
	if (Mix_OpenAudio(kFrequency, MIX_DEFAULT_FORMAT, kChannels,
			  kChunkSize) < 0) {
		throw SDL2pp::Exception("Mix_OpenAudio");
	}

	for (int i = 0; i < kSoundCount; ++i) {
		const SoundSpec& spec = kSounds[static_cast<std::size_t>(i)];
		if (spec.category != SoundCategory::Sfx) {
			continue;
		}
		const std::string path = core::soundPath(spec.file);
		Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
		if (chunk == nullptr) {
			Mix_CloseAudio();
			throw SDL2pp::Exception("Mix_LoadWAV");
		}
		chunks_[static_cast<std::size_t>(i)] = chunk;
	}
}

AudioPlayer::~AudioPlayer() {
	for (Mix_Chunk* chunk : chunks_) {
		Mix_FreeChunk(chunk);
	}
	Mix_CloseAudio();
}

void AudioPlayer::play(SoundId id) {
	Mix_Chunk* chunk = chunks_[static_cast<std::size_t>(id)];
	if (chunk == nullptr) {
		return;
	}
	Mix_PlayChannel(-1, chunk, 0);
}

}
