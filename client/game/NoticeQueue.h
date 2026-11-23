#pragma once

#include <optional>
#include <string>
#include <vector>

#include "game/LocalWorldView.h"

namespace game {

// El aviso vivo en pantalla: su id y los ticks que le quedan.
struct ActiveNotice {
    std::string id;
    int remainingTicks;
};

// Lleva la cuenta del aviso visible. Presentación PURA (sin SDL): recibe
// los eventos que manda la fuente (LocalWorldView::notices) y decide qué
// aviso hay y cuánto le queda. Hay UNO a la vez: uno nuevo reemplaza al
// anterior y repetir el mismo reinicia el contador (no apila carteles).
class NoticeQueue {
public:
    explicit NoticeQueue(int durationTicks);

    void show(const std::string& id);

    void observe(const std::vector<ViewNotice>& notices);

    // Descuenta el timer; al llegar a 0 el aviso se va.
    void tick();

    const std::optional<ActiveNotice>& active() const;

private:
    int durationTicks_;
    std::optional<ActiveNotice> active_;
};

}  // namespace game
