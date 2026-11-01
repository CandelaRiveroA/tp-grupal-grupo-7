#pragma once

namespace game {

// Datos del puntero sobre la ventana, en píxeles de pantalla.

// La posición actual del puntero (se mueve con el mouse).
struct PointerPosition {
    int x = 0;
    int y = 0;
};

// Un click del puntero sobre la ventana, en píxeles de pantalla. DTO
// plano: datos sin lógica. El significado del click (qué hay debajo y
// qué hacer) lo resuelven las capas de arriba; acá solo importa dónde.
struct PointerClick {
    int x = 0;
    int y = 0;
};

}  // namespace game
