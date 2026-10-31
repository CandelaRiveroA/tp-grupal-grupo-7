#include "InputManager.h"

#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>

namespace sdl {

InputManager::InputManager(int slotCount)
	: keymap_(defaultBindings()), slotCount_(slotCount) {
	commandBindings_ = defaultCommandBindings();
	// Prioridad al leer varias teclas a la vez: left, right, up, down
	// (el mismo orden que usaba la lectura original del input).
	directionBindings_ = {{
		{Action::Left, world::Direction::Left},
		{Action::Right, world::Direction::Right},
		{Action::Up, world::Direction::Back},
		{Action::Down, world::Direction::Front},
	}};
}

bool InputManager::up() const { return flags_[static_cast<std::size_t>(Action::Up)]; }

bool InputManager::down() const { return flags_[static_cast<std::size_t>(Action::Down)]; }

bool InputManager::left() const { return flags_[static_cast<std::size_t>(Action::Left)]; }

bool InputManager::right() const { return flags_[static_cast<std::size_t>(Action::Right)]; }

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

	// El click es la pulsación del botón primario del puntero. Otros
	// botones todavía no tienen significado: si algún día lo tienen,
	// es vocabulario nuevo que entra acá.
	if (event.type == SDL_MOUSEBUTTONDOWN &&
	    event.button.button == SDL_BUTTON_LEFT) {
		clicks_.push_back(game::PointerClick{event.button.x, event.button.y});
		return;
	}

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
		if (const game::InputCommand* command = commandFor(sym)) {
			fire(*command);
			return;
		}
	}

	setKey(sym, event.type == SDL_KEYDOWN);
}

auto InputManager::defaultCommandBindings() const
	-> std::unordered_map<SDL_Keycode, game::InputCommand> {

	using Kind = game::InputCommand::Kind;
	std::unordered_map<SDL_Keycode, game::InputCommand> bindings;
	// Orden = prioridad: ante colisiones de tecla, gana el primer comando.
	bindings.insert({SDLK_SPACE, game::InputCommand{Kind::UseTool, 0}});
	bindings.insert({SDLK_RETURN, game::InputCommand{Kind::Collect, 0}});
	for (int i = 0; i < slotCount_; ++i) {
		bindings.insert({static_cast<SDL_Keycode>(SDLK_1 + i),
				 game::InputCommand{Kind::SelectSlot, i}});
	}
	return bindings;
}

auto InputManager::commandFor(SDL_Keycode key) const -> const game::InputCommand* {
	if (const auto it = commandBindings_.find(key);
	    it != commandBindings_.end()) {
		return &it->second;
	}
	return nullptr;
}

void InputManager::fire(const game::InputCommand& command) {
	pending_.push_back(command);
}

std::vector<game::InputCommand> InputManager::takeCommands() {
	std::vector<game::InputCommand> commands(pending_.begin(), pending_.end());
	pending_.clear();
	return commands;
}

std::vector<game::PointerClick> InputManager::takeClicks() {
	std::vector<game::PointerClick> clicks;
	clicks.swap(clicks_);
	return clicks;
}

std::optional<world::Direction> InputManager::pressedDirection() const {
	for (const DirectionBinding& binding : directionBindings_) {
		if (flags_[static_cast<std::size_t>(binding.action)]) {
			return binding.direction;
		}
	}
	return std::nullopt;
}

void InputManager::setKey(SDL_Keycode key, bool pressed) {

	if (auto it = keymap_.find(key); it != keymap_.end()) {
		flags_[static_cast<std::size_t>(it->second)] = pressed;
	}
}

}
