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
        // Alterna entre caminar 1 y 2 celdas por movimiento.
        ToggleWalking,
        // Cambio de escenario local (tecla M): alterna mercado y
        // dormitorio. Lo consume PlaySession; NO viaja al GameClient.
        ToggleMarket,
        // DEMO (tecla O): pasa a la escena siguiente (mapas del editor
        // y salas de scenes.toml). Lo consume PlaySession; NO viaja al
        // GameClient. Se borra cuando el server mande el mapa.
        CycleMap,
        // DEMO (tecla B): abre/cierra el catálogo de Pierre en cualquier
        // escena (provisorio). Lo consume PlaySession; NO viaja al
        // GameClient. El catálogo real lo manda el server.
        OpenCatalog,
        // DEMO (tecla N): rota el estilo del catálogo del comerciante. Lo
        // consume PlaySession; NO viaja al GameClient. Se borra cuando el
        // servidor rote los estilos.
        NextMarketSet,
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
        // Revivir (V): la animación de muerte de los enemigos, al revés
        // (la acción Revive). Se va con el disparador.
        TestRevive,
        TestProjectileAdvance,
        TestProjectileDespawn,
        // Hace que el Skull ataque con el fuego VERDE (tecla V); el
        // proyectil despues se mueve/borra con J/L como el azul.
        TestGreenFire,
        // Abrir el chatbox (tecla T).
        OpenChat,
        // Con el chat abierto: Enter manda y ESC descarta.
        ChatSubmit,
        ChatCancel,
        // Abre/cierra el popup de configuración (ESC fuera del chat). Lo
        // consume PlaySession; NO viaja al GameClient.
        ToggleSettings,
    };

    Kind kind;

    // Índice de slot (0..slotCount-1). Solo es relevante para SelectSlot.
    int slot = 0;
};

}  // namespace game
