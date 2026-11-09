#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "game/AnimalSystem.h"
#include "game/EnemyBot.h"
#include "game/EnemyTestTrigger.h"
#include "game/FloorItems.h"
#include "game/GameClient.h"
#include "game/InteractionSession.h"
#include "game/LocalWorldView.h"
#include "world/CharacterCatalog.h"
#include "world/ContentIds.h"
#include "world/CreatureCatalog.h"
#include "world/Grid.h"

namespace game {

// La capa que SUPERPONE el mundo simulado del cliente (hoy animales de
// granja, enemigos e ítems del piso; mañana cultivos, ...) sobre la
// fuente de estado real (el server por red, el mock en tests). Es el
// "server falso" de los dominios que el server real todavía no tiene:
// cuando los tenga, se borra el módulo y solo cambia quién llena la
// vista (contrato §6).
//
// SOLO orquesta (convención §9): los sistemas de dominio (AnimalSystem,
// ...) traen sus reglas — los que atienden el toque implementan
// Interactor; las mecánicas compartidas son FloorItems (el piso del
// mundo) e InteractionSession (la interacción en curso). Sumar un
// sistema nuevo (cultivos, ...) = su módulo + registrarlo en
// `interactors_`.
class MockWorldClient: public GameClient, public EnemyTestTrigger {
public:
    // `spawnOnly`: si no está vacío, el mundo simulado spawnea SOLO esas
    // especies (p. ej. al envolver el cliente de red, que ya trae
    // gallinas y vacas, acá van solo los enemigos). Vacío = todas.
    MockWorldClient(std::unique_ptr<GameClient> inner, const world::CharacterCatalog& characters,
                    world::GridSpan grid,
                    std::vector<world::CreatureKind> spawnOnly = {});

    void sendCommand(const protocol::ClientMessage& message) override;
    void update(int iteration) override;
    const LocalWorldView& worldState() const override;
    bool connected() const override;

    // EnemyTestTrigger (andamiaje de prueba): guarda la acción pendiente;
    // se aplica al comienzo del próximo update (la última tecla del
    // frame gana).
    void triggerAll(world::ActionKind action) override;

private:
    // El primer interactuable que encuentre algo cerca del jugador
    // atiende el toque (lista en orden de prioridad). Devuelve si esta
    // capa consumió la intención.
    bool startInteraction();

    // Celda del jugador en la vista del inner (centro del mundo si
    // todavía no hay entidades).
    void playerCell(int& cellX, int& cellY) const;

    void mergeView();

    std::unique_ptr<GameClient> inner_;
    world::GridSpan grid_;
    // Especies que spawnea esta capa (vacío = todas).
    std::vector<world::CreatureKind> spawnOnly_;
    // Catálogo prestado por su dueño (lo superpone en vida): de ahí
    // salen los datos de las acciones para medir las ventanas.
    const world::CharacterCatalog* characters_;
    world::ContentIds contentIds_;

    // Mecánicas compartidas del mundo simulado.
    FloorItems floor_;
    InteractionSession session_;

    // Sistemas de dominio + el registro de interactuables (en orden de
    // prioridad): el primero que encuentra algo cerca atiende el toque.
    AnimalSystem animals_;
    // Los enemigos del mundo simulado (ANDAMIAJE TEMPORAL): su bot es
    // el stand-in del server hasta que mande enemigos y sus acciones.
    EnemyBot enemies_;
    std::vector<Interactor*> interactors_;
    // Quién atiende la interacción en curso (para su efecto final).
    Interactor* activeInteractor_ = nullptr;
    // Acción de prueba pedida por el disparador y todavía no aplicada
    // (se aplica al comienzo del próximo update; la última gana).
    std::optional<world::ActionKind> pendingTestAction_;

    // Sonidos de los sistemas del último update (per-tick).
    std::vector<ViewSoundEvent> soundEvents_;

    // Vista fusionada: la del inner + lo que aporta esta capa. Se arma
    // en update() (la del último update, como pide el contrato).
    LocalWorldView merged_;
};

}  // namespace game
