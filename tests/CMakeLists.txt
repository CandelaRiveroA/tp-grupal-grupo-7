target_sources(taller_tests
    PRIVATE
    # .cpp files
    index_cycle.cpp
    invite_code_format.cpp
    protocol.cpp
    # IndexCycle no tiene dependencias (ni Qt): el target de tests lo compila
    # directo en vez de linkear el cliente.
    ${PROJECT_SOURCE_DIR}/client/qt/util/IndexCycle.cpp
    PUBLIC
    # .h files
    )
