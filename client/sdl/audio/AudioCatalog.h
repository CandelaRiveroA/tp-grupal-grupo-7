#pragma once

#include <array>

namespace sdl {


enum class SoundCategory { Sfx, Music };

enum class SoundId { Step, RoosterCrow, ChickenCluck, CowMoo };

inline constexpr int kSoundCount = static_cast<int>(SoundId::CowMoo) + 1;

struct SoundSpec {
	const char* file;
	SoundCategory category;
};

inline constexpr std::array<SoundSpec, kSoundCount> kSounds = {{
	 {"sfx/movement/step.ogg", SoundCategory::Sfx},
	 {"sfx/farm/rooster.ogg", SoundCategory::Sfx},
	 {"sfx/farm/chicken_cluck.ogg", SoundCategory::Sfx},
	 {"sfx/farm/cow.ogg", SoundCategory::Sfx},
}};

}
