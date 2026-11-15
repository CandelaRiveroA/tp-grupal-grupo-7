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
        // Demo del desmayo (tecla F por ahora): presentación local
        // mientras el server no manda la acción. Lo consume PlaySession;
        // NO viaja al GameClient.
        Faint,
        // Abrir el chatbox (tecla T).
        OpenChat,
        // Con el chat abierto: Enter manda y ESC descarta.
        ChatSubmit,
        ChatCancel,
        // Demo (tecla M): simula que llegó un mensaje de otro jugador;
        // entra por el mismo embudo que usará la red.
        SimulateChatMessage,
        // ANDAMIAJE TEMPORAL (prueba): disparan acciones de los
        // enemigos del mundo simulado (G mover, F atacar, H daño, K
        // muerte), de sus proyectiles (J avanzar, L desaparecer) y el
        // fuego verde (V hacerlo aparecer). Los consume PlaySession con
        // su tabla de despacho; se van cuando el server mande esas
        // acciones.
        TestMove,
        TestAttack,
        TestDamage,
        TestDeath,
        TestProjectileAdvance,
        TestProjectileDespawn,
        // Hace que el Skull ataque con el fuego VERDE (tecla V); el
        // proyectil despues se mueve/borra con J/L como el azul.
        TestGreenFire,
    };

    Kind kind;

    // Índice de slot (0..slotCount-1). Solo es relevante para SelectSlot.
    int slot = 0;
};

}  // namespace game
