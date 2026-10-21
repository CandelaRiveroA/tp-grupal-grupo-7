#pragma once

#include <array>
#include <optional>
#include <unordered_map>
#include <vector>

#include <SDL2/SDL_events.h>

namespace sdl {

// Estado de las acciones de movimiento, alimentado por los eventos SDL.
// Reemplaza al core::InputManager que dependía de Qt::Key.
class InputManager {
public:

	enum class Action { Up, Down, Left, Right, Count };

	InputManager();

	void handleEvent(const SDL_Event& event);
	void setKey(SDL_Keycode key, bool pressed);

	void clearAll();

	bool up() const;
	bool down() const;
	bool left() const;
	bool right() const;

	const std::unordered_map<SDL_Keycode, Action>& bindings() const;
	bool isBound(SDL_Keycode key) const;
	std::optional<Action> actionFor(SDL_Keycode key) const;
	std::vector<SDL_Keycode> keysFor(Action action) const;

	void rebind(SDL_Keycode oldKey, SDL_Keycode newKey);

private:

	std::unordered_map<SDL_Keycode, Action> defaultBindings() const;

	std::unordered_map<SDL_Keycode, Action> keymap_;

	std::array<bool, static_cast<std::size_t>(Action::Count)> flags_{};
};

}
