target_sources(taller_tests PRIVATE 
    inventory_test.cpp
    world_test.cpp
    ../server/domain/inventory/item.cpp
    ../server/domain/inventory/product.cpp
    ../server/domain/inventory/slot.cpp
    ../server/domain/inventory/inventory.cpp
    ../server/domain/entities/player.cpp
    ../server/domain/entities/i_entity.cpp
    ../server/domain/map/world.cpp
    ../server/domain/map/cell.cpp
)