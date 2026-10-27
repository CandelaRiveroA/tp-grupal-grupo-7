#pragma once

#include <array>

namespace sdl {


enum class SoundCategory { Sfx, Music };

enum class SoundId { Step, RoosterCrow };

inline constexpr int kSoundCount = static_cast<int>(SoundId::RoosterCrow) + 1;

struct SoundSpec {
	const char* file;
	SoundCategory category;
};

inline constexpr std::array<SoundSpec, kSoundCount> kSounds = {{
	 {"sfx/movement/step.ogg", SoundCategory::Sfx},
	 {"sfx/farm/rooster.ogg", SoundCategory::Sfx},
}};

}
