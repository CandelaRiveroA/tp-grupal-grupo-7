target_sources(taller_tests
    PRIVATE
    # .cpp files
    character_layers.cpp
    index_cycle.cpp
    invite_code_format.cpp
    protocol.cpp
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
    # IndexCycle y CharacterLayers no tienen dependencias (ni Qt): el target
    # de tests los compila directo en vez de linkear el cliente.
    ${PROJECT_SOURCE_DIR}/client/qt/util/CharacterLayers.cpp
    ${PROJECT_SOURCE_DIR}/client/qt/util/IndexCycle.cpp
    PUBLIC
    # .h files
    )
