#ifndef SERVER_WORLD_VIEW_H
#define SERVER_WORLD_VIEW_H

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "domain/map/world.h"

#include "common_server_status.h"

// Lo que cada cliente ya conoce del mundo. sync() devuelve solo los cambios
// desde la última vez que se le envió estado a ese jugador, así que el
// primer sync de un jugador equivale a mandarle el mundo completo.
class WorldView {
public:
    std::vector<ServerMessage> sync(const World& world, uint8_t player_id);

    void forget(uint8_t player_id);

private:
    struct Position {
        uint16_t x;
        uint16_t y;

        bool operator<(const Position& other) const {
            return std::make_pair(y, x) < std::make_pair(other.y, other.x);
        }
    };

    struct PlayerPose {
        uint16_t x;
        uint16_t y;
        Direction direction;

        bool operator==(const PlayerPose& other) const {
            return x == other.x && y == other.y && direction == other.direction;
        }
    };

    struct CellContent {
        bool is_resource;
        std::string content_id;
        ResourceDamage damage;

        bool operator==(const CellContent& other) const {
            return is_resource == other.is_resource && content_id == other.content_id &&
                   damage == other.damage;
        }
    };

    struct SlotContent {
        std::string content_id;
        uint8_t count;

        bool operator==(const SlotContent& other) const {
            return content_id == other.content_id && count == other.count;
        }
    };

    struct PlayerView {
        bool synced = false;
        std::map<uint8_t, PlayerPose> poses;
        std::map<Position, CellContent> cells;
        std::vector<SlotContent> slots;
    };

    std::map<uint8_t, PlayerView> views;

    void sync_poses(const World& world, PlayerView& view, std::vector<ServerMessage>& out);
    void sync_cells(const World& world, PlayerView& view, std::vector<ServerMessage>& out);
    void sync_inventory(const World& world, uint8_t player_id, PlayerView& view,
                        std::vector<ServerMessage>& out);
};

#endif  // SERVER_WORLD_VIEW_H
