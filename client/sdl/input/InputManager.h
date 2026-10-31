#pragma once

#include <array>
#include <deque>
#include <optional>
#include <unordered_map>

#include <SDL2/SDL_events.h>

#include "game/InputCommand.h"

namespace sdl {

class InputManager {
public:

	enum class Action { Up, Down, Left, Right, Count };

	// slotCount: cuántas teclas numéricas (1..slotCount) seleccionan slots.
	// Debe coincidir con core::kInventorySlots; lo pasa quien construye.
	explicit InputManager(int slotCount);

	void handleEvent(const SDL_Event& event);

	bool up() const;
	bool down() const;
	bool left() const;
	bool right() const;

	// Saca la próxima intención one-shot (una por pulsación). Si se
	// pulsaron varias teclas en el mismo frame quedan varias pendientes y
	// salen en orden de pulsación. Se consume al leer.
	std::optional<game::InputCommand> takeCommand();

private:

	std::unordered_map<SDL_Keycode, Action> defaultBindings() const;

	// Despacho de comandos por diccionario: tecla -> comando one-shot.
	std::unordered_map<SDL_Keycode, game::InputCommand> defaultCommandBindings() const;

	const game::InputCommand* commandFor(SDL_Keycode key) const;

	void fire(const game::InputCommand& command);

	void setKey(SDL_Keycode key, bool pressed);

	std::unordered_map<SDL_Keycode, Action> keymap_;

	std::unordered_map<SDL_Keycode, game::InputCommand> commandBindings_;

	std::array<bool, static_cast<std::size_t>(Action::Count)> flags_{};

	int slotCount_ = 0;
	std::deque<game::InputCommand> pending_;  // cola de one-shot: varios por frame
};

}
