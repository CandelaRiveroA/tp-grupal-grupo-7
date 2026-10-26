target_sources(taller_tests
    PRIVATE
    # .cpp files
    character_layers.cpp
    index_cycle.cpp
    invite_code_format.cpp
    protocol.cpp
    # IndexCycle y CharacterLayers no tienen dependencias (ni Qt): el target
    # de tests los compila directo en vez de linkear el cliente.
    ${PROJECT_SOURCE_DIR}/client/qt/util/CharacterLayers.cpp
    ${PROJECT_SOURCE_DIR}/client/qt/util/IndexCycle.cpp
    PUBLIC
    # .h files
    )
