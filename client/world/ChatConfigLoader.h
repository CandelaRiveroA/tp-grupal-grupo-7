#pragma once

#include <string>

#include "world/ChatLayout.h"

namespace world {

// Carga ui/chatbox.toml en un ChatLayout; lanza ConfigError si es inválido.
class ChatConfigLoader {
public:
	ChatLayout loadFromFile(const std::string& path) const;
};

}
