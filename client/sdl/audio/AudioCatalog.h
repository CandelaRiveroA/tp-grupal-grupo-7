#pragma once

#include <array>

namespace sdl {


enum class SoundCategory { Sfx, Music };

enum class SoundId {
	Step,
	RoosterCrow,
	ToolSwitch,
	MineralHit,
	MineralCrack,
	MineralObtained,
	ChickenCluck,
	CowMoo,
	ShovelDig,
	WateringCanPour,
	Death,
};

inline constexpr int kSoundCount =
	static_cast<int>(SoundId::Death) + 1;

struct SoundSpec {
	const char* file;
	SoundCategory category;
};

inline constexpr std::array<SoundSpec, kSoundCount> kSounds = {{
	 {"sfx/movement/step.ogg", SoundCategory::Sfx},
	 {"sfx/farm/rooster.ogg", SoundCategory::Sfx},
	 {"sfx/ui/change_tool.ogg", SoundCategory::Sfx},
	 {"sfx/tools/hammer.ogg", SoundCategory::Sfx},
	 {"sfx/mining/brick2.ogg", SoundCategory::Sfx},
	 {"sfx/mining/discoverMineral.ogg", SoundCategory::Sfx},
	 {"sfx/farm/chicken_cluck.ogg", SoundCategory::Sfx},
	 {"sfx/farm/cow.ogg", SoundCategory::Sfx},
	 {"sfx/tools/shovel.ogg", SoundCategory::Sfx},
	 {"sfx/tools/watering_can.ogg", SoundCategory::Sfx},
	 {"sfx/player/death.ogg", SoundCategory::Sfx},
}};

}
