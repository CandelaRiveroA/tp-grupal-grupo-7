#pragma once

#include <array>
#include <optional>
#include <unordered_map>
#include <vector>

#include <SDL2/SDL_events.h>

namespace sdl {

class InputManager {
public:

	enum class Action { Up, Down, Left, Right, Count };

	InputManager();

	// slotCount: cuántas teclas numéricas (1..slotCount) seleccionan slots.
	// Debe coincidir con core::kInventorySlots; lo pasa quien construye.
	explicit InputManager(int slotCount);

	void handleEvent(const SDL_Event& event);
	void setKey(SDL_Keycode key, bool pressed);

	void clearAll();

	bool up() const;
	bool down() const;
	bool left() const;
	bool right() const;

	bool takeUseTool();

	// Retorna true una sola vez por pulsación de Enter (one-shot): es la
	// intención de RECOGER un item del piso. Se consume al leer.
	bool takeCollect();

	// Retorna el índice de slot (0..kInventorySlots-1) si se presionó
	// una tecla numérica 1-4, o nullopt si no. Se consume al leer (one-shot).
	std::optional<int> takeSlotSelect();

	const std::unordered_map<SDL_Keycode, Action>& bindings() const;
	bool isBound(SDL_Keycode key) const;
	std::optional<Action> actionFor(SDL_Keycode key) const;
	std::vector<SDL_Keycode> keysFor(Action action) const;

	void rebind(SDL_Keycode oldKey, SDL_Keycode newKey);

	void rebindUseTool(SDL_Keycode newKey);

	void rebindCollect(SDL_Keycode newKey);

private:

	std::unordered_map<SDL_Keycode, Action> defaultBindings() const;

	std::unordered_map<SDL_Keycode, Action> keymap_;

	std::array<bool, static_cast<std::size_t>(Action::Count)> flags_{};

	SDL_Keycode useToolKey_;
	bool useToolTriggered_ = false;
	SDL_Keycode collectKey_;
	bool collectTriggered_ = false;
	int slotCount_ = 0;
	std::optional<int> slotSelected_;  // índice 0..slotCount_-1, one-shot
};

}
