#pragma once

#include <optional>

namespace game {

// Decide cuándo avisa el chat: cuando llegó algún mensaje de otro jugador
// desde la última vez. Estado puro (sin SDL), alimentado por la vista
// (LocalWorldView::incomingChatVersion); qué suena lo elige quien lo usa.
class ChatCue {
public:
    // La primera llamada solo fija la referencia (lo que ya había al
    // entrar no avisa). Varios mensajes entre dos llamadas avisan una vez.
    bool update(int incomingChatVersion);

private:
    std::optional<int> seen_;
};

}  // namespace game
