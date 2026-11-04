#pragma once

#include <optional>
#include <string>

#include "world/CharacterAppearance.h"
#include "world/ItemKind.h"

namespace game {

// Un objetivo de interacción: DATOS de qué se hace sobre qué. Lo
// produce cada sistema del mundo cuando hay algo tocable cerca del
// jugador.
struct InteractionTarget {
    world::ActionKind action;
    // Vueltas completas de la animación que dura la interacción.
    int cycles = 1;
    // contentId que se obtiene al terminar.
    std::string reward;
    // Qué queda en las manos del jugador durante la acción (si algo).
    std::optional<world::ItemKind> held;
    // Contenido genérico sostenido durante la acción (contentId; ej. la
    // cosecha o el huevo). La sesión lo pasa a ViewEntity::activeContent.
    std::optional<std::string> heldContent;
    // Identidad del objetivo para que SU sistema aplique los efectos
    // (el id de la vaca, el índice del ítem, la parcela...).
    int targetId = -1;
    // Dónde está el objetivo: el jugador lo mira mientras interactúa.
    int cellX = 0;
    int cellY = 0;
};

// Un sistema del mundo con cosas tocables: encuentra su objetivo más
// cercano a una celda y aplica sus efectos al empezar/terminar la
// interacción. Sumar un sistema nuevo (cultivos, enemigos, ...) es
// implementar esta interfaz y registrarlo en el orquestador: los demás
// sistemas y la mecánica compartida no se tocan (convención §9,
// abierto/cerrado).
class Interactor {
public:
    virtual ~Interactor() = default;

    // El objetivo válido más cerca de la celda, si lo hay. Cada
    // sistema define su propia regla de cercanía y validez.
    virtual std::optional<InteractionTarget> findTargetNear(int cellX, int cellY) const = 0;

    // La interacción arrancó (ej: el huevo sale del piso).
    virtual void onInteractionStarted(const InteractionTarget& target) = 0;

    // La interacción terminó (ej: la vaca deja de estar lista).
    virtual void onInteractionFinished(const InteractionTarget& target) = 0;
};

}  // namespace game
