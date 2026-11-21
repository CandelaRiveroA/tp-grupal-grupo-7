#pragma once

#include <map>
#include <utility>
#include <vector>

#include "game/ActionStartCue.h"
#include "sdl/audio/AudioCatalog.h"
#include "world/CharacterAppearance.h"
#include "world/CharacterCatalog.h"

namespace core {

// Cuándo suena el sonido de una acción (vocabulario de presentación,
// convención §4). `Start` = al empezar la acción (el caso histórico);
// `LastFrame` = recién al entrar al ÚLTIMO cuadro de la animación (el
// hechizo del vampiro suena con el gesto final, no con el arranque).
enum class ActionSoundMoment {
	Start,
	LastFrame,
};

// Fila de la tabla de sonidos por acción.
struct ActionSound {
	sdl::SoundId sound;
	ActionSoundMoment moment;
};

// Qué suena al EMPEZAR cada acción de cada criatura (el daño recibido
// del jugador suena distinto al de un enemigo). Combina el detector de
// flanco (game::ActionStartCue: una vez por inicio) con la tabla
// (criatura, acción) -> sonido y su momento. Un par sin fila no suena.
// Sumar el sonido de una acción = una fila (convención §12). Espejo de
// ToolUseSounds, que es por herramienta en vez de por acción.
//
// El momento `LastFrame` se agenda desde los MISMOS datos de la acción
// (frames x frame_ticks de characters.toml, contrato §10): si se cambia
// el ritmo del ataque, el sonido sigue sincronizado. Si la acción se
// interrumpe antes del último cuadro, el sonido pendiente se descarta.
class ActionSounds {
public:
	// characters: los datos de las acciones (para el momento LastFrame).
	explicit ActionSounds(const world::CharacterCatalog& characters);

	// view = la vista de este tick (la de presentación: así también
	// suenan las acciones locales). Devuelve los sonidos a reproducir
	// en este tick; si varias entidades emiten el MISMO sonido, suena
	// una sola vez.
	std::vector<sdl::SoundId> update(const game::LocalWorldView& view,
					 int iteration);

private:
	using ActionKey = std::pair<world::CreatureKind, world::ActionKind>;

	// Sonido agendado por un frame futuro (momento LastFrame).
	struct PendingSound {
		int entityId;
		world::ActionKind action;
		int dueIteration;
		sdl::SoundId sound;
	};

	// Tick en que la animación entra a su último cuadro, relativo al
	// inicio de la acción (0 si la acción no tiene datos: suena de una).
	int lastFrameTick(const game::ActionStart& start) const;

	const world::CharacterCatalog& characters_;
	game::ActionStartCue cue_;
	std::map<ActionKey, ActionSound> sounds_;
	std::vector<PendingSound> pending_;
};

}  // namespace core
