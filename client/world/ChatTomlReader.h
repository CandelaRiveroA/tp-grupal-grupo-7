#pragma once

#include "world/ChatLayout.h"

namespace tomlcfg {
class TomlDocument;
}

namespace world {

// Lee ui/chatbox.toml en un ChatLayout; lanza ConfigError si la
// geometría es inválida.
class ChatTomlReader {
public:
	explicit ChatTomlReader(const tomlcfg::TomlDocument& doc);

	ChatLayout read() const;

private:
	const tomlcfg::TomlDocument& doc_;
};

}
