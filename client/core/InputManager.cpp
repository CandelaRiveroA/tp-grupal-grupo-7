#include "InputManager.h"

namespace core {

InputManager::InputManager() : keymap_(defaultBindings()) {}

bool InputManager::up() const { return flags_[static_cast<std::size_t>(Action::Up)]; }

bool InputManager::down() const { return flags_[static_cast<std::size_t>(Action::Down)]; }

bool InputManager::left() const { return flags_[static_cast<std::size_t>(Action::Left)]; }

bool InputManager::right() const { return flags_[static_cast<std::size_t>(Action::Right)]; }

auto InputManager::bindings() const -> const std::unordered_map<Qt::Key, Action>& {
	return keymap_;
}

bool InputManager::isBound(Qt::Key key) const { return keymap_.count(key) > 0; }

auto InputManager::defaultBindings() const -> std::unordered_map<Qt::Key, Action> {

	return {
		{Qt::Key_Up, Action::Up},
		{Qt::Key_W, Action::Up},
		{Qt::Key_Down, Action::Down},
		{Qt::Key_S, Action::Down},
		{Qt::Key_Left, Action::Left},
		{Qt::Key_A, Action::Left},
		{Qt::Key_Right, Action::Right},
		{Qt::Key_D, Action::Right},
	};
}

void InputManager::setKey(Qt::Key key, bool pressed) {

	if (auto it = keymap_.find(key); it != keymap_.end()) {
		flags_[static_cast<std::size_t>(it->second)] = pressed;
	}
}

void InputManager::clearAll() {
	flags_.fill(false);
}

auto InputManager::actionFor(Qt::Key key) const -> std::optional<Action> {
	if (auto it = keymap_.find(key); it != keymap_.end()) {
		return it->second;
	}
	return std::nullopt;
}

auto InputManager::keysFor(Action action) const -> std::vector<Qt::Key> {
	std::vector<Qt::Key> keys;
	for (const auto& [key, act] : keymap_) {
		if (act == action) {
			keys.push_back(key);
		}
	}
	return keys;
}

void InputManager::rebind(Qt::Key oldKey, Qt::Key newKey) {
	if (oldKey == newKey) {
		return;
	}
	if (auto it = keymap_.find(oldKey); it != keymap_.end()) {
		keymap_[newKey] = it->second;
		keymap_.erase(it);
	}

}

}
