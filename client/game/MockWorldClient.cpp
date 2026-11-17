#include "game/MockWorldClient.h"

#include <string>
#include <utility>
#include <variant>

#include "game/GameConstants.h"

namespace game {

MockWorldClient::MockWorldClient(std::unique_ptr<GameClient> inner,
                                 const world::CharacterCatalog& characters,
                                 const world::GridSpan grid,
                                 std::vector<world::CreatureKind> spawnOnly):
        inner_(std::move(inner)),
        grid_(grid),
        spawnOnly_(std::move(spawnOnly)),
        characters_(&characters),
        animals_(grid, floor_),
        enemies_(characters, grid),
        interactors_{&animals_, &floor_} {
    int playerX = grid_.columns / 2;
    int playerY = grid_.rows / 2;
    playerCell(playerX, playerY);
    animals_.spawnAnimals(characters, playerX, playerY, spawnOnly_);
    enemies_.spawnEnemies(characters, playerX, playerY, spawnOnly_);
    mergeView();
}

void MockWorldClient::sendCommand(const protocol::ClientMessage& message) {
    // El toque (Take) con un objetivo válido y CERCANO arranca la
    // interacción en esta capa (el mundo simulado es suyo). Todo lo
    // demás va a la fuente real.
    if (std::holds_alternative<protocol::TakeMessage>(message) && startInteraction()) {
        return;
    }
    inner_->sendCommand(message);
}

void MockWorldClient::update(const int iteration) {
    inner_->update(iteration);

    int playerX = grid_.columns / 2;
    int playerY = grid_.rows / 2;
    playerCell(playerX, playerY);
    animals_.update(iteration, playerX, playerY);
    soundEvents_ = animals_.takeSoundEvents();

    // La acción de prueba pedida desde el input se aplica recién acá
    // (al comienzo del update que la observa): así la ventana observada
    // coincide con frames x frameTicks y la última tecla del frame gana.
    if (pendingTestAction_.has_value()) {
        if (pendingAttackProjectile_.has_value()) {
            enemies_.triggerAttackWith(*pendingAttackProjectile_, iteration, playerX, playerY);
        } else {
            enemies_.triggerAll(*pendingTestAction_, iteration, playerX, playerY);
        }
        pendingTestAction_.reset();
        pendingAttackProjectile_.reset();
    }
    enemies_.update(iteration);

    // La ventana se cierra en este tick: el sistema dueño del objetivo
    // aplica su efecto final.
    if (session_.advance() && activeInteractor_ != nullptr) {
        activeInteractor_->onInteractionFinished(session_.target());
        activeInteractor_ = nullptr;
    }

    mergeView();
}

const LocalWorldView& MockWorldClient::worldState() const { return merged_; }

bool MockWorldClient::connected() const { return inner_->connected(); }

void MockWorldClient::sendChat(const std::string& text) { inner_->sendChat(text); }

void MockWorldClient::receiveChat(const std::uint8_t playerId, const std::string& characterName,
                                  const std::string& text) {
    inner_->receiveChat(playerId, characterName, text);
}

void MockWorldClient::triggerAll(const world::ActionKind action) {
    // Solo la anota (andamiaje de prueba): se aplica al comienzo del
    // próximo update. La última tecla del frame gana.
    pendingTestAction_ = action;
    pendingAttackProjectile_.reset();
}

void MockWorldClient::triggerAttackWith(const world::CreatureKind projectile) {
    // El ataque es la acción Attack, pero con un proyectil concreto. La
    // última tecla del frame gana (pisa cualquier acción pendiente).
    pendingTestAction_ = world::ActionKind::Attack;
    pendingAttackProjectile_ = projectile;
}

void MockWorldClient::advanceProjectiles() { enemies_.advanceProjectiles(); }

void MockWorldClient::despawnProjectiles() { enemies_.despawnProjectiles(); }

bool MockWorldClient::startInteraction() {
    const world::CreatureData* player = characters_->find(world::CreatureKind::Player);
    if (player == nullptr) {
        return false;  // sin datos del jugador no hay animación posible
    }
    int playerX = 0;
    int playerY = 0;
    playerCell(playerX, playerY);

    for (Interactor* interactor: interactors_) {
        const std::optional<InteractionTarget> target =
                interactor->findTargetNear(playerX, playerY);
        if (!target.has_value()) {
            continue;
        }
        session_.start(*target, playerX, playerY, *player);
        interactor->onInteractionStarted(*target);
        activeInteractor_ = interactor;
        return true;
    }
    return false;
}

void MockWorldClient::playerCell(int& cellX, int& cellY) const {
    const LocalWorldView& state = inner_->worldState();
    if (state.entities.empty()) {
        return;
    }
    cellX = state.entities[kLocalPlayerIndex].cellX;
    cellY = state.entities[kLocalPlayerIndex].cellY;
}

void MockWorldClient::mergeView() {
    merged_ = inner_->worldState();

    animals_.appendEntities(merged_.entities);
    enemies_.appendEntities(merged_.entities);
    merged_.soundEvents.insert(merged_.soundEvents.end(), soundEvents_.begin(), soundEvents_.end());

    // Los ítems del piso del mundo, al final de los del inner. El
    // "server falso" marca lo que es interactivo (hoy los huevos).
    for (const FloorItem& item: floor_.items()) {
        const std::string* contentId = contentIds_.find(item.kind);
        merged_.droppedItems.push_back(ViewDroppedItem{contentId != nullptr ? *contentId : "",
                                                       item.cellX, item.cellY, true});
    }

    if (!merged_.entities.empty()) {
        session_.mergePlayer(merged_.entities[kLocalPlayerIndex]);
    }
    session_.mergeRewards(merged_);
}

}  // namespace game
