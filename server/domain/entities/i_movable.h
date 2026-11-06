#ifndef IMOVABLE_H
#define IMOVABLE_H

#include <utility>  // cpplint: lo pide por la declaración de move

#include "../../../common/common_constants.h"

class World;

// Capacidades de movimiento: un paso por llamada en la dirección dada.
class IMovable {
public:
    virtual ~IMovable() = default;

    // Intenta dar una casilla en `dir`. Devuelve false si no se movió
    // (destino bloqueado o borde del mapa).
    virtual bool move(World& w, Direction dir) = 0;
};

#endif  // IMOVABLE_H
