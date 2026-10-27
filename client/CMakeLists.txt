# Qt es dependencia SOLO del cliente (server y tests no la usan)
find_package(Qt6 COMPONENTS Widgets REQUIRED)

# Qt6::Widgets queda definido como importado LOCAL de este subdir, pero el
# target taller_client se crea en la raíz y el generador resuelve
# $<TARGET_LINKER_FILE:...> desde el scope de la raíz. Hacerlo global desde
# acá (único lugar donde se puede setear) evita "No target Qt6::Widgets".
set_target_properties(Qt6::Widgets PROPERTIES IMPORTED_GLOBAL TRUE)

# Ordenados por carpeta: core/ (composición), qt/, game/, world/, toml/
# y sdl/ (con sus subcarpetas por responsabilidad: context, loop, render,
# input, audio, log).
target_sources(taller_client
    PRIVATE
    # .cpp files
    main.cpp
    core/Application.cpp
    core/Constants.cpp
    core/StartupScreens.cpp
    core/WindowConfig.cpp
    qt/LobbyWindow.cpp
    world/Scenario.cpp
    sdl/context/SdlContext.cpp
    sdl/loop/GameLoop.cpp
    sdl/render/Renderer.cpp
    sdl/render/ElementPainter.cpp
    sdl/render/SheetCache.cpp
    sdl/render/SpriteSheet.cpp
    sdl/render/CharacterSprite.cpp
    sdl/render/InventoryHud.cpp
    sdl/render/ObtainedBannerHud.cpp
    sdl/input/InputManager.cpp
    sdl/audio/AudioPlayer.cpp
    PUBLIC
    # .h files (los headers con Q_OBJECT van acá: AUTOMOC los necesita)
    core/Application.h
    core/Constants.h
    core/SessionConfig.h
    core/StartupSequence.h
    core/StartupScreens.h
    core/WindowConfig.h
    qt/LobbyWindow.h
    game/FootstepCue.h
    game/ResourceAudioCue.h
    game/ObtainedBannerQueue.h
    game/GameClient.h
    game/LocalWorldView.h
    game/MockGameClient.h
    game/MoveCommand.h
    game/ViewMotion.h
    game/WalkPacer.h
    game/ActionResolver.h
    game/InventorySelector.h
    world/CharacterAppearance.h
    world/CharacterCatalog.h
    world/CharacterCatalogLoader.h
    world/CharacterTomlReader.h
    world/Color.h
    world/ConfigError.h
    world/CreatureCatalog.h
    world/EntityAppearance.h
    world/Grid.h
    world/Scenario.h
    world/InventoryLayout.h
    world/InventoryTomlReader.h
    world/InventoryConfigLoader.h
    world/ItemCatalog.h
    world/ToolOverlay.h
    world/ToolOverlayLoader.h
    world/ToolOverlayTomlReader.h
    world/ResourceCatalog.h
    world/ResourceConfigLoader.h
    world/ResourceTomlReader.h
    world/ObtainedBannerCatalog.h
    world/ObtainedBannerConfigLoader.h
    world/ObtainedBannerTomlReader.h
    world/ObtainedBannerLayout.h
    world/SceneConfigLoader.h
    world/SceneDescription.h
    world/SceneKeys.h
    world/ScenePixelMapper.h
    world/SceneTomlReader.h
    toml/TomlDocument.h
    sdl/context/SdlConstants.h
    sdl/context/SdlContext.h
    sdl/loop/FrameClock.h
    sdl/loop/GameLoop.h
    sdl/render/Renderer.h
    sdl/render/ElementPainter.h
    sdl/render/SheetCache.h
    sdl/render/SpriteSheet.h
    sdl/render/TileLayout.h
    sdl/render/CharacterSprite.h
    sdl/render/InventoryHud.h
    sdl/render/ObtainedBannerHud.h
    sdl/input/InputManager.h
    sdl/audio/AudioCatalog.h
    sdl/audio/AudioPlayer.h
    sdl/log/Logger.h
    )

# AUTOMOC solo para este target (no afecta a server ni a tests)
set_target_properties(taller_client PROPERTIES AUTOMOC ON)

# La copia de assets al build (add_custom_command POST_BUILD) vive en la
# raíz: el target taller_client se crea ahí y CMake solo permite esa
# forma del comando en el directorio donde se creó el target.
target_compile_definitions(taller_client PRIVATE
    # \" produce comillas literales: el macro llega como "…/assets"
    TALLER_ASSET_DIR=\"${CMAKE_BINARY_DIR}/assets\"
    TALLER_SOUNDS_DIR=\"${CMAKE_BINARY_DIR}/sounds\")

# La raíz solo expone el repo root como include ("common/foo.h");
# con esto los includes del cliente son "core/...", "qt/...", etc.
target_include_directories(taller_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Qt6::Widgets es un target importado definido en este subdir (por el
# find_package de arriba): hay que linkearlo con forma keyword (PRIVATE)
# para que CMake lo resuelva desde acá y no desde la raíz.
target_link_libraries(taller_client PRIVATE Qt6::Widgets)
