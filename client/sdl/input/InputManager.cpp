#include "InputManager.h"

#include <stdexcept>

#include <SDL2/SDL_keycode.h>

namespace sdl {

InputManager::InputManager()
	: InputManager(0) {}

InputManager::InputManager(int slotCount)
	: keymap_(defaultBindings()), useToolKey_(SDLK_SPACE),
	  collectKey_(SDLK_RETURN), slotCount_(slotCount) {
	commandBindings_ = defaultCommandBindings();
}

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

	const SDL_Keycode sym = event.key.keysym.sym;

	// Los comandos one-shot se disparan solo con la pulsación (keydown);
	// la suelta no dispara nada (cae al mapeo de movimiento, que la ignora
	// si la tecla no está bound).
	if (event.type == SDL_KEYDOWN) {
		if (const Command* command = commandFor(sym)) {
			fire(*command);
			return;
		}
	}

	setKey(sym, event.type == SDL_KEYDOWN);
}

auto InputManager::defaultCommandBindings() const
	-> std::unordered_map<SDL_Keycode, Command> {

	std::unordered_map<SDL_Keycode, Command> bindings;
	// Orden = prioridad: ante colisiones de tecla, gana el primer comando.
	bindings.insert({useToolKey_, Command{CommandKind::UseTool, 0}});
	bindings.insert({collectKey_, Command{CommandKind::Collect, 0}});
	for (int i = 0; i < slotCount_; ++i) {
		bindings.insert({static_cast<SDL_Keycode>(SDLK_1 + i),
				 Command{CommandKind::SelectSlot, i}});
	}
	return bindings;
}

auto InputManager::commandFor(SDL_Keycode key) const -> const Command* {
	if (const auto it = commandBindings_.find(key);
	    it != commandBindings_.end()) {
		return &it->second;
	}
	return nullptr;
}

void InputManager::fire(const Command& command) {
	switch (command.kind) {
		case CommandKind::UseTool:
			useToolTriggered_ = true;
			break;
		case CommandKind::Collect:
			collectTriggered_ = true;
			break;
		case CommandKind::SelectSlot:
			slotSelected_ = command.slot;
			break;
		default:
			throw std::logic_error("input: comando desconocido");
	}
}

bool InputManager::takeUseTool() {
	const bool triggered = useToolTriggered_;
	useToolTriggered_ = false;
	return triggered;
}

bool InputManager::takeCollect() {
	const bool triggered = collectTriggered_;
	collectTriggered_ = false;
	return triggered;
}

std::optional<int> InputManager::takeSlotSelect() {
	const std::optional<int> selected = slotSelected_;
	slotSelected_ = std::nullopt;
	return selected;
}

void InputManager::rebindUseTool(SDL_Keycode newKey) {
	commandBindings_.erase(useToolKey_);
	useToolKey_ = newKey;
	commandBindings_.insert_or_assign(newKey,
					  Command{CommandKind::UseTool, 0});
}

void InputManager::rebindCollect(SDL_Keycode newKey) {
	commandBindings_.erase(collectKey_);
	collectKey_ = newKey;
	commandBindings_.insert_or_assign(newKey,
					  Command{CommandKind::Collect, 0});
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
