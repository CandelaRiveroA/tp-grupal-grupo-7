#include "core/ActionSounds.h"

#include <set>

namespace core {

ActionSounds::ActionSounds(const world::CharacterCatalog& characters)
	: characters_(characters)
	, sounds_{
		  // El jugador recibe daño: la titilación suena como golpe
		  // al jugador (una vez por daño, al empezar la acción).
		  {{world::CreatureKind::Player, world::ActionKind::Damage},
		   {sdl::SoundId::CharacterHit, ActionSoundMoment::Start}},
		  // El jugador desmaya (vida o energía en 0): la caída suena
		  // una sola vez, al empezar el one-shot terminal.
		  {{world::CreatureKind::Player, world::ActionKind::Faint},
		   {sdl::SoundId::Death, ActionSoundMoment::Start}},
		  // Enemigos recibiendo daño: todos comparten el golpe a
		  // enemigo (una fila por criatura; sin fila no suena).
		  {{world::CreatureKind::Orc, world::ActionKind::Damage},
		   {sdl::SoundId::EnemyHit, ActionSoundMoment::Start}},
		  {{world::CreatureKind::Skeleton, world::ActionKind::Damage},
		   {sdl::SoundId::EnemyHit, ActionSoundMoment::Start}},
		  {{world::CreatureKind::Vampire, world::ActionKind::Damage},
		   {sdl::SoundId::EnemyHit, ActionSoundMoment::Start}},
		  {{world::CreatureKind::Skull, world::ActionKind::Damage},
		   {sdl::SoundId::EnemyHit, ActionSoundMoment::Start}},
		  // Enemigos muriendo: el crujido de monstruo muerto. La
		  // acción Death es terminal (se queda en el último frame),
		  // así que el flanco suena UNA sola vez por muerte.
		  {{world::CreatureKind::Orc, world::ActionKind::Death},
		   {sdl::SoundId::MonsterDead, ActionSoundMoment::Start}},
		  {{world::CreatureKind::Skeleton, world::ActionKind::Death},
		   {sdl::SoundId::MonsterDead, ActionSoundMoment::Start}},
		  {{world::CreatureKind::Vampire, world::ActionKind::Death},
		   {sdl::SoundId::MonsterDead, ActionSoundMoment::Start}},
		  {{world::CreatureKind::Skull, world::ActionKind::Death},
		   {sdl::SoundId::MonsterDead, ActionSoundMoment::Start}},
		  // Enemigos atacando: cada uno suena con su arma. Orco y
		  // esqueleto reutilizan el tajo de la espada AL EMPEZAR; el
		  // vampiro suena con el gesto FINAL del ataque (LastFrame:
		  // el hechizo sale cuando está por terminar); el Skull, la
		  // bola de fuego al empezar.
		  {{world::CreatureKind::Orc, world::ActionKind::Attack},
		   {sdl::SoundId::SwordSwing, ActionSoundMoment::Start}},
		  {{world::CreatureKind::Skeleton, world::ActionKind::Attack},
		   {sdl::SoundId::SwordSwing, ActionSoundMoment::Start}},
		  {{world::CreatureKind::Vampire, world::ActionKind::Attack},
		   {sdl::SoundId::ColdSpell, ActionSoundMoment::LastFrame}},
		  {{world::CreatureKind::Skull, world::ActionKind::Attack},
		   {sdl::SoundId::Fireball, ActionSoundMoment::Start}},
	  } {}

int ActionSounds::lastFrameTick(const game::ActionStart& start) const {
	const world::CreatureData& creature = characters_.dataFor(start.kind);
	const world::DirectionAppearance& dir =
		creature.directions[static_cast<int>(start.facing)];
	const auto action = dir.actions.find(start.action);
	if (action == dir.actions.end() || action->second.frames.empty()) {
		return 0;  // sin datos de la acción: suena al empezar
	}
	const world::ActionData& data = action->second;
	const int frames = static_cast<int>(data.frames.size());
	return (frames - 1) * data.frameTicks;
}

std::vector<sdl::SoundId> ActionSounds::update(
	const game::LocalWorldView& view, const int iteration) {
	std::vector<sdl::SoundId> sounds;
	std::set<sdl::SoundId> alreadyQueued;

	// Sonidos agendados (LastFrame): suenan al llegar su tick, pero
	// solo si la acción sigue viva. Si la entidad cambió de acción
	// (el ataque se cortó antes del último cuadro) o desapareció, el
	// pendiente se descarta.
	for (auto it = pending_.begin(); it != pending_.end();) {
		const game::ViewEntity* entity = nullptr;
		for (const game::ViewEntity& candidate : view.entities) {
			if (candidate.id == it->entityId) {
				entity = &candidate;
				break;
			}
		}
		if (entity == nullptr || entity->action != it->action) {
			it = pending_.erase(it);
			continue;
		}
		if (iteration >= it->dueIteration) {
			if (alreadyQueued.insert(it->sound).second) {
				sounds.push_back(it->sound);
			}
			it = pending_.erase(it);
			continue;
		}
		++it;
	}

	// Flancos de este tick: los de Start suenan de una; los de
	// LastFrame se agendan para el último cuadro de la animación.
	for (const game::ActionStart& start : cue_.update(view)) {
		const auto sound = sounds_.find({start.kind, start.action});
		if (sound == sounds_.end()) {
			continue;
		}
		if (sound->second.moment == ActionSoundMoment::Start) {
			if (alreadyQueued.insert(sound->second.sound).second) {
				sounds.push_back(sound->second.sound);
			}
			continue;
		}
		pending_.push_back(PendingSound{
			start.id, start.action,
			iteration + lastFrameTick(start), sound->second.sound});
	}
	return sounds;
}

}  // namespace core
