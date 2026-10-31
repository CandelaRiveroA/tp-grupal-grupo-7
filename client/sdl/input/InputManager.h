#pragma once

#include <array>
#include <deque>
#include <optional>
#include <unordered_map>

#include <SDL2/SDL_events.h>

namespace sdl {

class InputManager {
public:

	enum class Action { Up, Down, Left, Right, Count };

	enum class CommandKind { UseTool, Collect, SelectSlot };

	// DTO plano: la intención one-shot que dispara una tecla.
	struct Command {
		CommandKind kind;
		int slot = 0;
	};

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
	std::optional<Command> takeCommand();

private:

	std::unordered_map<SDL_Keycode, Action> defaultBindings() const;

	// Despacho de comandos por diccionario: tecla -> comando one-shot.
	std::unordered_map<SDL_Keycode, Command> defaultCommandBindings() const;

	const Command* commandFor(SDL_Keycode key) const;

	void fire(const Command& command);

	void setKey(SDL_Keycode key, bool pressed);

	std::unordered_map<SDL_Keycode, Action> keymap_;

	std::unordered_map<SDL_Keycode, Command> commandBindings_;

	std::array<bool, static_cast<std::size_t>(Action::Count)> flags_{};

	int slotCount_ = 0;
	std::deque<Command> pending_;  // cola de one-shot: varios por frame
};

}
