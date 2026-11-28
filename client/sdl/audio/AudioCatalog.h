#pragma once

#include <array>

#include "world/SoundCategory.h"

namespace sdl {

using SoundCategory = world::SoundCategory;

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
	SwordSwing,
	CharacterHit,
	EnemyHit,
	MonsterDead,
	ColdSpell,
	Fireball,
	NewMessage,
	Cancel,
	ChestOpen,
	ChestClose,
};

inline constexpr int kSoundCount =
	static_cast<int>(SoundId::ChestClose) + 1;

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
	 {"sfx/tools/sword.ogg", SoundCategory::Sfx},
	 {"sfx/combat/characterHit.ogg", SoundCategory::Sfx},
	 {"sfx/combat/hitEnemy.ogg", SoundCategory::Sfx},
	 {"sfx/combat/monsterdead.ogg", SoundCategory::Sfx},
	 {"sfx/combat/coldSpell.ogg", SoundCategory::Sfx},
	 {"sfx/combat/fireball.ogg", SoundCategory::Sfx},
	 {"sfx/ui/new_message.ogg", SoundCategory::Sfx},
	 {"sfx/ui/cancel.ogg", SoundCategory::Sfx},
	 {"sfx/tools/doorCreak.ogg", SoundCategory::Sfx},
	 {"sfx/tools/doorCreakReverse.ogg", SoundCategory::Sfx},
}};

}
