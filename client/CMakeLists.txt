# Qt es dependencia SOLO del cliente (server y tests no la usan)
find_package(Qt6 COMPONENTS Widgets REQUIRED)

# Qt6::Widgets queda definido como importado LOCAL de este subdir, pero el
# target taller_client se crea en la raíz y el generador resuelve
# $<TARGET_LINKER_FILE:...> desde el scope de la raíz. Hacerlo global desde
# acá (único lugar donde se puede setear) evita "No target Qt6::Widgets".
set_target_properties(Qt6::Widgets PROPERTIES IMPORTED_GLOBAL TRUE)

target_sources(taller_client
    PRIVATE
    # .cpp files
    main.cpp
    core/Application.cpp
    core/WindowConfig.cpp
    qt/LobbyWindow.cpp
    qt/resources/resources.qrc
    qt/screens/MainMenuScreen.cpp
    qt/screens/Screen.cpp
    qt/services/SaveService.cpp
    qt/services/SessionService.cpp
    qt/services/WorldService.cpp
    qt/util/IndexCycle.cpp
    sdl/GameLoop.cpp
    sdl/InputManager.cpp
    sdl/SdlContext.cpp
    sdl/Renderer.cpp
    world/Player.cpp
    world/Scenario.cpp
    PUBLIC
    # .h files (los headers con Q_OBJECT van acá: AUTOMOC los necesita)
    core/Application.h
    core/SessionConfig.h
    core/WindowConfig.h
    qt/LobbyWindow.h
    qt/screens/MainMenuScreen.h
    qt/screens/Screen.h
    qt/services/SaveService.h
    qt/services/ServiceError.h
    qt/services/SessionService.h
    qt/services/WorldService.h
    qt/util/IndexCycle.h
    sdl/GameLoop.h
    sdl/InputManager.h
    sdl/SdlContext.h
    sdl/Renderer.h
    world/Player.h
    world/Scenario.h
    )

# AUTOMOC y AUTORCC solo para este target (no afectan a server ni a tests)
set_target_properties(taller_client PROPERTIES AUTOMOC ON AUTORCC ON)

# La raíz solo expone el repo root como include ("common/foo.h");
# con esto los includes del cliente son "core/...", "qt/...", etc.
target_include_directories(taller_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Qt6::Widgets es un target importado definido en este subdir (por el
# find_package de arriba): hay que linkearlo con forma keyword (PRIVATE)
# para que CMake lo resuelva desde acá y no desde la raíz.
target_link_libraries(taller_client PRIVATE Qt6::Widgets)

# Cliente de consola (prueba de red sin GUI): binario aparte para no
# duplicar main() con el taller_client.
add_executable(taller_client_cli client_main.cpp client.cpp)
target_link_libraries(taller_client_cli PRIVATE taller_common)
target_include_directories(taller_client_cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
