#pragma once

#include <array>
#include <optional>
#include <unordered_map>
#include <vector>

#include <QtCore/qnamespace.h>

namespace core {

class InputManager {
public:

	enum class Action { Up, Down, Left, Right, Count };

	InputManager();

	void setKey(Qt::Key key, bool pressed);

	void clearAll();

	bool up() const;
	bool down() const;
	bool left() const;
	bool right() const;

	const std::unordered_map<Qt::Key, Action>& bindings() const;
	bool isBound(Qt::Key key) const;
	std::optional<Action> actionFor(Qt::Key key) const;
	std::vector<Qt::Key> keysFor(Action action) const;

	void rebind(Qt::Key oldKey, Qt::Key newKey);

private:

	std::unordered_map<Qt::Key, Action> defaultBindings() const;

	std::unordered_map<Qt::Key, Action> keymap_;

	std::array<bool, static_cast<std::size_t>(Action::Count)> flags_{};
};

}
