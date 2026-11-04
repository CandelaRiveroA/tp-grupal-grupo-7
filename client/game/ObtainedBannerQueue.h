#pragma once

#include <string>
#include <vector>

#include "game/LocalWorldView.h"

namespace game {

// Un cartel vivo en la pila. stackIndex 0 = arriba de todo; crece hacia
// abajo. remainingTicks baja hasta 0 y entonces el cartel se descarta.
struct ActiveBanner {
    std::string contentId;
    int remainingTicks;
    int stackIndex;
};

// Mantiene la pila de carteles de "contenido obtenido" con sus timers. Es
// presentación PURA (sin SDL): recibe los eventos de obtención que manda
// la fuente (no los deduce) y decide qué carteles hay y en qué orden.
// Análogo a ResourceAudioCue/FootstepCue: decide, no dibuja.
//
// Contrato: se alimenta SOLO de LocalWorldView.obtained. Reemplazar el
// mock por el servidor no cambia esta clase.
class ObtainedBannerQueue {
public:
    // maxVisible: tope de la pila; si está llena, los eventos nuevos se
    // descartan.
    ObtainedBannerQueue(int durationTicks, int maxVisible);

    void push(const std::string& contentId);

    void observe(const std::vector<ViewObtained>& obtained);

    // Descuenta timers, descarta expirados y recompacta los stackIndex.
    void tick();

    const std::vector<ActiveBanner>& active() const;

private:
    void reindex();

    int durationTicks_;
    int maxVisible_;
    std::vector<ActiveBanner> banners_;
};

}  // namespace game
