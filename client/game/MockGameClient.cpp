#include "game/MockGameClient.h"

#include <algorithm>
#include <cstddef>

#include "world/CreatureCatalog.h"
#include "world/Grid.h"

namespace game {

MockGameClient::MockGameClient(int gridColumns, int gridRows, int startX, int startY,
                               int toolUseDurationTicks):
        gridColumns_(gridColumns),
        gridRows_(gridRows),
        playerCellX_(startX),
        playerCellY_(startY),
        playerFacing_(world::Direction::Front),
        playerMoving_(false),
        toolUseDurationTicks_(toolUseDurationTicks > 0 ? toolUseDurationTicks : 1) {
    placeMinerals();
    rebuildInventory();
    rebuildView();
}

void MockGameClient::placeMinerals() {
    // Piedras de prueba alrededor del jugador. El servidor real decidirá
    // qué y dónde; esto solo simula un mundo con minerales variados.
    const int cx = playerCellX_;
    const int cy = playerCellY_;
    const std::vector<MineralSpot> spots = {
            {"silver", cx + 1, cy, 0},
            {"coal", cx + 2, cy, 0},
            {"lapis", cx + 1, cy + 1, 0},
            {"gold", cx + 2, cy + 1, 0},
            {"coal", cx - 1, cy, 0},
            {"silver", cx, cy + 2, 0},
            {"gold", cx - 1, cy + 2, 0},
            {"lapis", cx + 3, cy, 0},
            // Rellenos destructibles (1 golpe, sin drop): una fila de
            // rocas 2 celdas arriba del jugador, para verlas de una.
            {"rock", cx - 2, cy - 2, 0, 1, false},
            {"rock_2", cx - 1, cy - 2, 0, 1, false},
            {"rock_3", cx, cy - 2, 0, 1, false},
            {"rock_4", cx + 1, cy - 2, 0, 1, false},
            {"rock_5", cx + 2, cy - 2, 0, 1, false},
            {"rock_6", cx + 3, cy - 2, 0, 1, false},
    };
    for (const MineralSpot& spot: spots) {
        if (spot.cellX < 0 || spot.cellX >= gridColumns_ || spot.cellY < 0 ||
            spot.cellY >= gridRows_) {
            continue;
        }
        minerals_.push_back(spot);
    }
}

void MockGameClient::sendCommand(const protocol::ClientMessage& message) {
    if (const auto* move = std::get_if<protocol::MoveMessage>(&message)) {
        toolUseRemaining_ = 0;
        walk(move->direction);
    } else if (const auto* pick = std::get_if<protocol::PickMessage>(&message)) {
        playerMoving_ = false;
        playerFacing_ = world::viewDirectionFrom(pick->direction);
        toolUseRemaining_ = toolUseDurationTicks_;
        // El daño se confirma al COMPLETAR la ventana (fin de la
        // animación), no al apretar, como hará el servidor.
        int targetX = 0;
        int targetY = 0;
        frontCell(pick->direction, targetX, targetY);
        pendingMineral_ = mineralAt(targetX, targetY);
    } else if (const auto* take = std::get_if<protocol::TakeMessage>(&message)) {
        // Recoger el item del piso en la celda del jugador. El
        // servidor validará; acá el mock decide si hay algo y, si lo
        // hay, programa el evento de obtención (panel + sonido).
        playerMoving_ = false;
        playerFacing_ = world::viewDirectionFrom(take->direction);
        collectAt(playerCellX_, playerCellY_);
    }
    rebuildView();
}

void MockGameClient::update(int) {
    view_.resourceHits.clear();
    advanceToolUse();
    rebuildView();
    advanceObtained();
}

const LocalWorldView& MockGameClient::worldState() const { return view_; }

void MockGameClient::frontCell(::Direction direction, int& cellX, int& cellY) const {
    cellX = playerCellX_;
    cellY = playerCellY_;
    switch (direction) {
        case ::Direction::UP:
            cellY -= 1;
            break;
        case ::Direction::DOWN:
            cellY += 1;
            break;
        case ::Direction::LEFT:
            cellX -= 1;
            break;
        case ::Direction::RIGHT:
            cellX += 1;
            break;
    }
}

int MockGameClient::mineralAt(int cellX, int cellY) const {
    for (std::size_t i = 0; i < minerals_.size(); ++i) {
        if (minerals_[i].cellX == cellX && minerals_[i].cellY == cellY) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void MockGameClient::walk(::Direction direction) {
    int targetX = 0;
    int targetY = 0;
    frontCell(direction, targetX, targetY);

    const int clampedX = std::clamp(targetX, 0, gridColumns_ - 1);
    const int clampedY = std::clamp(targetY, 0, gridRows_ - 1);

    playerMoving_ = (clampedX != playerCellX_ || clampedY != playerCellY_);
    playerCellX_ = clampedX;
    playerCellY_ = clampedY;
    playerFacing_ = world::viewDirectionFrom(direction);
}

void MockGameClient::advanceToolUse() {
    if (toolUseRemaining_ <= 0) {
        return;
    }
    --toolUseRemaining_;
    // Al completar el golpe (0) se confirma el daño al mineral apuntado.
    if (toolUseRemaining_ == 0 && pendingMineral_ >= 0 &&
        pendingMineral_ < static_cast<int>(minerals_.size())) {
        MineralSpot& spot = minerals_[static_cast<std::size_t>(pendingMineral_)];
        if (spot.hits < spot.hitsToBreak) {
            ++spot.hits;
            view_.resourceHits.push_back(
                    ViewResourceHit{spot.cellX, spot.cellY, spot.hits == spot.hitsToBreak});
            // Al agotar el mineral que dropea, CAE al piso como item
            // recogible (las rocas no dejan nada). Ya no se obtiene
            // automático: el jugador debe pararse sobre la celda y mandar
            // Take (Enter).
            if (spot.hits == spot.hitsToBreak && spot.drops) {
                dropped_.push_back(DroppedItem{spot.contentId, spot.cellX, spot.cellY});
            }
        }
        pendingMineral_ = -1;
    }
}

void MockGameClient::advanceObtained() {
    view_.obtained.clear();
    // El evento de obtención lo dispara un Take exitoso (no se deduce):
    // se emite por un solo tick y luego se limpia.
    if (obtainedPending_) {
        view_.obtained.push_back(ViewObtained{obtainedContentId_});
        obtainedPending_ = false;
    }
}

int MockGameClient::droppedAt(int cellX, int cellY) const {
    for (std::size_t i = 0; i < dropped_.size(); ++i) {
        if (dropped_[i].cellX == cellX && dropped_[i].cellY == cellY) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void MockGameClient::collectAt(int cellX, int cellY) {
    const int index = droppedAt(cellX, cellY);
    if (index < 0) {
        return;  // no hay nada para recoger en esa celda
    }
    const std::string contentId = dropped_[static_cast<std::size_t>(index)].contentId;
    obtainedContentId_ = contentId;
    obtainedPending_ = true;
    dropped_.erase(dropped_.begin() + index);
    // Recoger = sumar al inventario. El servidor manda la cantidad; el
    // mock la simula acumulando +1.
    addToInventory(contentId);
}

void MockGameClient::addToInventory(const std::string& contentId) {
    view_.addInventoryContent(contentId);
}

void MockGameClient::rebuildView() {
    view_.entities.clear();
    view_.grid = world::GridSpan{gridColumns_, gridRows_};

    // El jugador es SIEMPRE la entidad 0 con id estable 0.
    ViewEntity player;
    player.kind = world::CreatureKind::Player;
    player.id = 0;
    player.cellX = playerCellX_;
    player.cellY = playerCellY_;
    player.facing = playerFacing_;
    player.moving = playerMoving_;
    if (toolUseRemaining_ > 0) {
        player.action = world::ActionKind::UseTool;
        player.activeTool = world::ItemKind::Pickaxe;
    } else {
        player.action = playerMoving_ ? world::ActionKind::Walk : world::ActionKind::Idle;
    }
    view_.entities.push_back(player);

    rebuildResources();
    rebuildDropped();
}

void MockGameClient::rebuildDropped() {
    view_.droppedItems.clear();
    for (const DroppedItem& item: dropped_) {
        ViewDroppedItem view;
        // El mock simula minerales: su contentId es el mismo vocabulario
        // genérico que el inventario. El servidor mandará el contentId.
        view.contentId = item.contentId;
        view.cellX = item.cellX;
        view.cellY = item.cellY;
        view_.droppedItems.push_back(view);
    }
}

void MockGameClient::rebuildResources() {
    view_.resources.clear();
    // Mismo criterio que el servidor (world_view.cpp): 0 intacto; el golpe
    // previo al final se dibuja broken; el resto cracked. Agotado, sale
    // de la vista.
    for (const MineralSpot& spot: minerals_) {
        if (spot.hits >= spot.hitsToBreak) {
            continue;  // agotado
        }
        world::ResourceState state;
        if (spot.hits == 0) {
            state = world::ResourceState::Intact;
        } else if (spot.hits + 1 >= spot.hitsToBreak) {
            state = world::ResourceState::Broken;
        } else {
            state = world::ResourceState::Cracked;
        }
        ViewResource resource;
        resource.contentId = spot.contentId;
        resource.cellX = spot.cellX;
        resource.cellY = spot.cellY;
        resource.state = state;
        view_.resources.push_back(resource);
    }
}

void MockGameClient::rebuildInventory() {
    // El inventario es genérico: cada slot lleva un contentId (string) y
    // una cantidad. Las herramientas arrancan con cantidad 1 (sin número).
    view_.inventory.clear();
    view_.inventory.push_back({0, "pickaxe", 1});
    view_.inventory.push_back({1, "shovel", 1});
    view_.inventory.push_back({2, "watering_can", 1});
    view_.inventory.push_back({3, "sword", 1});
}

}  // namespace game
