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
        // Atajos de PRUEBA (teclas T, C y N): fuerzan el momento del día
        // (atardecer, anochecer, noche) sin esperar la hora del server.
        // Lo normal es que lo decida el reloj del server (ver
        // DayPhase::setClock); el atajo dura hasta la próxima etapa.
        TestDusk,
        TestEvening,
        TestNight,
    };

    Kind kind;

    // Índice de slot (0..slotCount-1). Solo es relevante para SelectSlot.
    int slot = 0;
};

}  // namespace game
