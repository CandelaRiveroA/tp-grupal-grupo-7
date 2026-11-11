#pragma once

#include <array>
#include <deque>
#include <optional>
#include <unordered_map>
#include <vector>

#include <SDL2/SDL_events.h>

#include "game/InputCommand.h"
#include "game/PointerClick.h"
#include "world/ViewDirection.h"

namespace sdl {

class InputManager {
public:

	enum class Action { Up, Down, Left, Right, Count };

	// slotCount: cuántos slots seleccionan las teclas de la tabla
	// slotKeys_ (una tecla por slot, en orden). Lo pasa quien construye
	// desde los datos del layout (InventoryLayout::count).
	explicit InputManager(int slotCount);

	void handleEvent(const SDL_Event& event);

	bool up() const;
	bool down() const;
	bool left() const;
	bool right() const;

	// Saca las intenciones one-shot pendientes (una por pulsación), en
	// orden de pulsación, y vacía la cola. Pueden salir varias juntas si
	// se pulsaron varias teclas en el mismo frame.
	std::vector<game::InputCommand> takeCommands();

	// Movimiento sostenido como dirección del juego: la primera dirección
	// de la tabla de prioridad que esté pulsada, o vacío si no hay
	// movimiento.
	std::optional<world::Direction> pressedDirection() const;

	// Saca los clicks del puntero pendientes (uno por pulsación de
	// botón), en orden, y vacía la cola. Varios clicks pueden salir
	// juntos si hubo varios en el mismo frame.
	std::vector<game::PointerClick> takeClicks();

	// Posición actual del puntero (px de pantalla), actualizada por el
	// movimiento del mouse.
	const game::PointerPosition& mousePosition() const;

private:

	std::unordered_map<SDL_Keycode, Action> defaultBindings() const;

	// Despacho de comandos por diccionario: tecla -> comando one-shot.
	std::unordered_map<SDL_Keycode, game::InputCommand> defaultCommandBindings() const;

	const game::InputCommand* commandFor(SDL_Keycode key) const;

	void fire(const game::InputCommand& command);

	void setKey(SDL_Keycode key, bool pressed);

	// Tabla de prioridad para leer el movimiento como dirección del juego.
	struct DirectionBinding {
		Action action;
		world::Direction direction;
	};

	std::unordered_map<SDL_Keycode, Action> keymap_;

	std::unordered_map<SDL_Keycode, game::InputCommand> commandBindings_;

	// Tabla de teclas que seleccionan slot (una por slot, en orden):
	// se enlazan las primeras slotCount. Agregar un slot es sumar una
	// tecla acá (hoy los dígitos 1..9 y 0); no hay fórmulas que rompan
	// fuera de rango.
	std::array<SDL_Keycode, 10> slotKeys_;

	std::array<bool, static_cast<std::size_t>(Action::Count)> flags_{};

	std::array<DirectionBinding, static_cast<std::size_t>(Action::Count)>
		directionBindings_;

	int slotCount_ = 0;
	std::deque<game::InputCommand> pending_;  // cola de one-shot: varios por frame
	std::vector<game::PointerClick> clicks_;  // clicks pendientes: varios por frame
	game::PointerPosition pointerPosition_{};
};

}
