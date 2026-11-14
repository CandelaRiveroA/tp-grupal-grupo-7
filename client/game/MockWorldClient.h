#pragma once

#include <memory>
#include <vector>

#include "game/AnimalSystem.h"
#include "game/FloorItems.h"
#include "game/GameClient.h"
#include "game/InteractionSession.h"
#include "game/LocalWorldView.h"
#include "world/CharacterCatalog.h"
#include "world/ContentIds.h"
#include "world/Grid.h"

namespace game {

// La capa que SUPERPONE el mundo simulado del cliente (hoy animales e
// ítems del piso; mañana cultivos, enemigos, ...) sobre la fuente de
// estado real (el server por red, el mock en tests). Es el "server
// falso" de los dominios que el server real todavía no tiene: cuando
// los tenga, se borra el módulo y solo cambia quién llena la vista
// (contrato §6).
//
// SOLO orquesta (convención §9): los sistemas de dominio (AnimalSystem,
// ...) implementan Interactor y traen sus reglas; las mecánicas
// compartidas son FloorItems (el piso del mundo) e InteractionSession
// (la interacción en curso). Sumar un sistema nuevo (cultivos,
// enemigos) = su módulo + registrarlo en `interactors_`.
class MockWorldClient: public GameClient {
public:
    MockWorldClient(std::unique_ptr<GameClient> inner, const world::CharacterCatalog& characters,
                    world::GridSpan grid);

    void sendCommand(const protocol::ClientMessage& message) override;
    void update(int iteration) override;
    const LocalWorldView& worldState() const override;
    bool connected() const override;

    // El chat no es suyo: lo delega en la fuente que envuelve (si no, se
    // perdería al pasar por esta capa).
    void sendChat(const std::string& text) override;
    void receiveChat(std::uint8_t playerId, const std::string& characterName,
                     const std::string& text) override;

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
    std::vector<Interactor*> interactors_;
    // Quién atiende la interacción en curso (para su efecto final).
    Interactor* activeInteractor_ = nullptr;

    // Sonidos de los sistemas del último update (per-tick).
    std::vector<ViewSoundEvent> soundEvents_;

    // Vista fusionada: la del inner + lo que aporta esta capa. Se arma
    // en update() (la del último update, como pide el contrato).
    LocalWorldView merged_;
};

}  // namespace game
