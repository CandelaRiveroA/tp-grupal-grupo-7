#pragma once

#include <map>
#include <optional>

#include "game/ToolUseCue.h"
#include "sdl/audio/AudioCatalog.h"
#include "world/ItemKind.h"

namespace core {

// Qué suena al USAR cada herramienta. Combina el detector de flanco
// (game::ToolUseCue: una vez por uso) con la tabla herramienta ->
// sonido. Una herramienta sin fila no suena (el pico ya suena por el
// impacto del recurso). Sumar el sonido de una herramienta = una fila.
class ToolUseSounds {
public:
	ToolUseSounds();

	// activeTool = la herramienta en uso este tick. Devuelve el sonido
	// a reproducir solo en el instante en que empieza el uso.
	std::optional<sdl::SoundId> update(
		const std::optional<world::ItemKind>& activeTool);

private:
	game::ToolUseCue cue_;
	std::map<world::ItemKind, sdl::SoundId> sounds_;
};

}  // namespace core
