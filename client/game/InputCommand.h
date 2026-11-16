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
        // ANDAMIAJE TEMPORAL (prueba): disparan acciones de los
        // enemigos del mundo simulado (G mover, F atacar, H daño, K
        // muerte) y de sus proyectiles (J avanzar el fuego, L
        // desaparecerlo). Los consume PlaySession con su tabla de
        // despacho; se van cuando el server mande esas acciones.
        TestMove,
        TestAttack,
        TestDamage,
        TestDeath,
        // Revivir (V): la animación de muerte de los enemigos, al revés
        // (la acción Revive). Se va con el disparador.
        TestRevive,
        TestProjectileAdvance,
        TestProjectileDespawn,
        // Gasto de energía simulado (tecla U por ahora): representa una
        // acción del juego que cansa al jugador. Lo consume PlaySession
        // contra game::FractionStat; NO viaja al GameClient.
        SpendEnergy,
        // Gasto de vida simulado (tecla I por ahora): mismo mecanismo
        // que la energía, con otro stat. Lo consume PlaySession; NO
        // viaja al GameClient.
        SpendLife,
        // Demo del desmayo (tecla O por ahora): presentación local
        // mientras el server no manda la acción. Lo consume PlaySession;
        // NO viaja al GameClient.
        Faint,
        // Demo del daño recibido (tecla R por ahora): titilación del
        // jugador (aparece/desaparece) como presentación local mientras
        // el server no mande el daño. Lo consume PlaySession con
        // game::LocalActionWindow; NO viaja al GameClient.
        Hurt,
        // Demo del charco de sangre (tecla C por ahora): deja un charco
        // en la celda del jugador. Lo consume PlaySession; NO viaja al
        // GameClient.
        BloodPuddle,
        // Abrir el chatbox (tecla T).
        OpenChat,
        // Con el chat abierto: Enter manda y ESC descarta.
        ChatSubmit,
        ChatCancel,
        // Demo (tecla M): simula que llegó un mensaje de otro jugador;
        // entra por el mismo embudo que usará la red.
        SimulateChatMessage,
    };

    Kind kind;

    // Índice de slot (0..slotCount-1). Solo es relevante para SelectSlot.
    int slot = 0;
};

}  // namespace game
