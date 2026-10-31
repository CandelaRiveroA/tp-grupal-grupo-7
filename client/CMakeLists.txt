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
    client_sender.cpp
    client_receiver.cpp
    game/NetworkGameClient.cpp
    core/Application.cpp
    core/Constants.cpp
    core/StartupScreens.cpp
    core/WindowConfig.cpp
    qt/LobbyWindow.cpp
    qt/resources/resources.qrc
    qt/screens/CharacterScreen.cpp
    qt/screens/JoinCodeScreen.cpp
    qt/screens/MainMenuScreen.cpp
    qt/screens/SaveListScreen.cpp
    qt/screens/Screen.cpp
    qt/screens/WorldSelectScreen.cpp
    qt/services/AssetCatalog.cpp
    qt/services/SaveService.cpp
    qt/services/SessionService.cpp
    qt/services/WorldService.cpp
    qt/services/fakes/FakeAssetCatalog.cpp
    qt/services/fakes/FakeSaveService.cpp
    qt/services/fakes/FakeSessionService.cpp
    qt/services/fakes/FakeWorldService.cpp
    qt/theme/GameCursor.cpp
    qt/theme/TitleBackdrop.cpp
    qt/theme/TitleSprites.cpp
    qt/util/CharacterLayers.cpp
    qt/util/IndexCycle.cpp
    qt/widgets/AsyncStateView.cpp
    qt/widgets/CharacterPreview.cpp
    qt/widgets/TitleButton.cpp
    world/Scenario.cpp
    sdl/context/SdlContext.cpp
    sdl/loop/GameLoop.cpp
    sdl/render/Renderer.cpp
    sdl/render/ElementPainter.cpp
    sdl/render/SheetCache.cpp
    sdl/render/SpriteSheet.cpp
    sdl/render/CharacterSprite.cpp
    sdl/render/InventoryHud.cpp
    sdl/input/InputManager.cpp
    sdl/audio/AudioPlayer.cpp
    sdl/audio/MusicPlayer.cpp
    PUBLIC
    # .h files (los headers con Q_OBJECT van acá: AUTOMOC los necesita)
    core/Application.h
    core/Constants.h
    core/MusicControl.h
    core/MusicTrack.h
    core/SessionConfig.h
    core/StartupSequence.h
    core/StartupScreens.h
    core/WindowConfig.h
    qt/LobbyWindow.h
    qt/screens/CharacterScreen.h
    qt/screens/JoinCodeScreen.h
    qt/screens/MainMenuScreen.h
    qt/screens/SaveListScreen.h
    qt/screens/Screen.h
    qt/screens/WorldSelectScreen.h
    qt/services/AssetCatalog.h
    qt/services/SaveService.h
    qt/services/ServiceError.h
    qt/services/SessionService.h
    qt/services/WorldService.h
    qt/services/fakes/FakeAssetCatalog.h
    qt/services/fakes/FakeLatency.h
    qt/services/fakes/FakeSaveService.h
    qt/services/fakes/FakeSessionService.h
    qt/services/fakes/FakeWorldService.h
    qt/theme/CloudRects.h
    qt/theme/CursorRects.h
    qt/theme/GameCursor.h
    qt/theme/TitleBackdrop.h
    qt/theme/TitleSpriteRects.h
    qt/theme/TitleSprites.h
    qt/util/CharacterLayer.h
    qt/util/CharacterLayers.h
    qt/util/IndexCycle.h
    qt/widgets/AsyncStateView.h
    qt/widgets/CharacterPreview.h
    qt/widgets/TitleButton.h
    client_sender.h
    client_receiver.h
    game/FootstepCue.h
    game/GameClient.h
    game/LocalWorldView.h
    game/NetworkGameClient.h
    game/MockGameClient.h
    game/MoveCommand.h
    game/ViewMotion.h
    game/WalkPacer.h
    game/ActionResolver.h
    game/ToolUse.h
    world/CharacterAppearance.h
    world/CharacterCatalog.h
    world/CharacterCatalogLoader.h
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
    sdl/input/InputManager.h
    sdl/audio/AudioCatalog.h
    sdl/audio/AudioConfig.h
    sdl/audio/AudioPlayer.h
    sdl/audio/MusicPlayer.h
    sdl/log/Logger.h
    )

# AUTOMOC y AUTORCC solo para este target (no afectan a server ni a tests)
set_target_properties(taller_client PROPERTIES AUTOMOC ON AUTORCC ON)

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
