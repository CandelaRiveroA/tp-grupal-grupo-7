#pragma once

namespace game {

// Intención one-shot del usuario sobre el juego (una por pulsación de
// tecla). DTO plano: datos sin lógica. Vive en game/ y no en sdl/ para
// que la lógica de juego pueda consumir input sin depender de SDL: la
// capa sdl/ traduce los eventos del dispositivo a estos comandos.
struct InputCommand {

    // El tipo de intención. Agregar una acción nueva es sumar un valor
    // acá y una entrada en el despacho por tabla que lo consume.
    enum class Kind {
        UseTool,
        Collect,
        SelectSlot,
        // Gasto de energía simulado (tecla G por ahora): representa una
        // acción del juego que cansa al jugador. Lo consume PlaySession
        // contra game::FractionStat; NO viaja al GameClient.
        SpendEnergy,
        // Gasto de vida simulado (tecla H por ahora): mismo mecanismo
        // que la energía, con otro stat. Lo consume PlaySession; NO
        // viaja al GameClient.
        SpendLife,
    };

    Kind kind;

    // Índice de slot (0..slotCount-1). Solo es relevante para SelectSlot.
    int slot = 0;
};

}  // namespace game
