#ifndef SERVER_COMMANDS_MERCHANT_COMMANDS_H
#define SERVER_COMMANDS_MERCHANT_COMMANDS_H

#include <cstdint>
#include <string>
#include <vector>

#include "domain/map/world.h"
 
// La implementacion (NPC comerciante, catalogo, compra/venta, mutacion de oro e
// inventario) queda para mas adelante; cuando se haga, estos handlers se
// registran en la misma tabla de comandos que los cheats.
namespace commands {

std::string consultar_catalogo(World& world, uint8_t player_id,
                              const std::vector<std::string>& args);
std::string comprar(World& world, uint8_t player_id,
                    const std::vector<std::string>& args);
std::string vender(World& world, uint8_t player_id,
                   const std::vector<std::string>& args);

}  // namespace commands

#endif  // SERVER_COMMANDS_MERCHANT_COMMANDS_H