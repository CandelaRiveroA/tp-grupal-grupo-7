#pragma once

namespace game {

// Un click del puntero sobre la ventana, en píxeles de pantalla. DTO
// plano: datos sin lógica. El significado del click (qué hay debajo y
// qué hacer) lo resuelven las capas de arriba; acá solo importa dónde.
struct PointerClick {
    int x = 0;
    int y = 0;
};

}  // namespace game
