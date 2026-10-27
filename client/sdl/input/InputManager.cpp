#include "InputManager.h"

#include <SDL2/SDL_keycode.h>

namespace sdl {

InputManager::InputManager()
	: keymap_(defaultBindings()), useToolKey_(SDLK_SPACE) {}

bool InputManager::up() const { return flags_[static_cast<std::size_t>(Action::Up)]; }

bool InputManager::down() const { return flags_[static_cast<std::size_t>(Action::Down)]; }

bool InputManager::left() const { return flags_[static_cast<std::size_t>(Action::Left)]; }

bool InputManager::right() const { return flags_[static_cast<std::size_t>(Action::Right)]; }

auto InputManager::bindings() const -> const std::unordered_map<SDL_Keycode, Action>& {
	return keymap_;
}

bool InputManager::isBound(SDL_Keycode key) const { return keymap_.count(key) > 0; }

auto InputManager::defaultBindings() const -> std::unordered_map<SDL_Keycode, Action> {

	return {
		{SDLK_UP, Action::Up},
		{SDLK_w, Action::Up},
		{SDLK_DOWN, Action::Down},
		{SDLK_s, Action::Down},
		{SDLK_LEFT, Action::Left},
		{SDLK_a, Action::Left},
		{SDLK_RIGHT, Action::Right},
		{SDLK_d, Action::Right},
	};
}

void InputManager::handleEvent(const SDL_Event& event) {

	if (event.type != SDL_KEYDOWN && event.type != SDL_KEYUP) {
		return;
	}

	if (event.key.repeat != 0) {
		return;
	}

	if (event.type == SDL_KEYDOWN && event.key.keysym.sym == useToolKey_) {
		useToolTriggered_ = true;
		return;
	}

	setKey(event.key.keysym.sym, event.type == SDL_KEYDOWN);
}

bool InputManager::takeUseTool() {
	const bool triggered = useToolTriggered_;
	useToolTriggered_ = false;
	return triggered;
}

void InputManager::rebindUseTool(SDL_Keycode newKey) {
	useToolKey_ = newKey;
}

void InputManager::setKey(SDL_Keycode key, bool pressed) {

	if (auto it = keymap_.find(key); it != keymap_.end()) {
		flags_[static_cast<std::size_t>(it->second)] = pressed;
	}
}

void InputManager::clearAll() {
	flags_.fill(false);
}

auto InputManager::actionFor(SDL_Keycode key) const -> std::optional<Action> {
	if (auto it = keymap_.find(key); it != keymap_.end()) {
		return it->second;
	}
	return std::nullopt;
}

auto InputManager::keysFor(Action action) const -> std::vector<SDL_Keycode> {
	std::vector<SDL_Keycode> keys;
	for (const auto& [key, act] : keymap_) {
		if (act == action) {
			keys.push_back(key);
		}
	}
	return keys;
}

void InputManager::rebind(SDL_Keycode oldKey, SDL_Keycode newKey) {
	if (oldKey == newKey) {
		return;
	}
	if (auto it = keymap_.find(oldKey); it != keymap_.end()) {
		keymap_[newKey] = it->second;
		keymap_.erase(it);
	}
}

}
