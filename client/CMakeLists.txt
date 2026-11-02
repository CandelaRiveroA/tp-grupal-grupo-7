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
    core/PlaySession.cpp
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
    qt/services/CharacterLayerSource.cpp
    qt/services/SaveService.cpp
    qt/services/SessionService.cpp
    qt/services/CharacterSpriteComposer.cpp
    qt/services/WorldService.cpp
    qt/services/fakes/FakeSaveService.cpp
    qt/services/fakes/FakeSessionService.cpp
    qt/services/fakes/FakeWorldService.cpp
    qt/theme/CharacterSheets.cpp
    qt/theme/GameCursor.cpp
    qt/theme/TitleBackdrop.cpp
    qt/theme/TitleSprites.cpp
    qt/util/CharacterLayers.cpp
    qt/util/IndexCycle.cpp
    qt/widgets/AsyncStateView.cpp
    qt/widgets/CharacterPreview.cpp
    qt/theme/JoinCodeSprites.cpp
    qt/theme/NineSlice.cpp
    qt/theme/SkinRecolorer.cpp
    qt/widgets/InputBox.cpp
    qt/widgets/ParchmentBanner.cpp
    qt/widgets/TitleButton.cpp
    world/Scenario.cpp
    sdl/context/SdlContext.cpp
    sdl/loop/GameLoop.cpp
    sdl/render/Renderer.cpp
    sdl/render/ElementPainter.cpp
    sdl/render/EntityRenderer.cpp
    sdl/render/SheetCache.cpp
    sdl/render/SpriteSheet.cpp
    sdl/render/CharacterSprite.cpp
    sdl/render/ToolOverlayPainter.cpp
    sdl/render/InventoryHud.cpp
    sdl/render/ObtainedBannerHud.cpp
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
    core/PlaySession.h
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
    qt/services/CharacterLayerSource.h
    qt/services/CharacterSpriteComposer.h
    qt/services/SaveService.h
    qt/services/ServiceError.h
    qt/services/SessionService.h
    qt/services/WorldService.h
    qt/services/fakes/FakeLatency.h
    qt/services/fakes/FakeSaveService.h
    qt/services/fakes/FakeSessionService.h
    qt/services/fakes/FakeWorldService.h
    qt/theme/CharacterRects.h
    qt/theme/CloudRects.h
    qt/theme/JoinCodeRects.h
    qt/theme/JoinCodeSprites.h
    qt/theme/CharacterSheetRects.h
    qt/theme/CharacterSheets.h
    qt/theme/NineSlice.h
    qt/theme/SkinRecolorer.h
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
    qt/widgets/InputBox.h
    qt/widgets/ParchmentBanner.h
    qt/widgets/TitleButton.h
    client_sender.h
    client_receiver.h
    game/FootstepCue.h
    game/ResourceAudioCue.h
    game/ObtainedBannerQueue.h
    game/GameClient.h
    game/GameConstants.h
    game/InputCommand.h
    game/LocalWorldView.h
    game/NetworkGameClient.h
    game/MockGameClient.h
    game/MoveCommand.h
    game/PlayerController.h
    game/ViewMotion.h
    game/WalkPacer.h
    game/ActionResolver.h
    game/InventorySelector.h
    world/CharacterAppearance.h
    world/CharacterCatalog.h
    world/CharacterCatalogLoader.h
    world/CharacterLayout.h
    world/WorldConstants.h
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
    world/ToolOverlayLayout.h
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
    world/ElementLayout.h
    world/ChunkGrid.h
    world/SceneElementIndex.h
    game/ViewSpatialIndex.h
    toml/TomlDocument.h
    sdl/context/SdlConstants.h
    sdl/context/SdlContext.h
    sdl/loop/FrameClock.h
    sdl/loop/GameLoop.h
    sdl/render/Renderer.h
    sdl/render/ElementPainter.h
    sdl/render/EntityRenderer.h
    sdl/render/SheetCache.h
    sdl/render/SpriteSheet.h
    sdl/render/TileLayout.h
    sdl/render/Viewport.h
    sdl/render/Camera.h
    sdl/render/CharacterSprite.h
    sdl/render/RenderConstants.h
    sdl/render/ToolOverlayPainter.h
    sdl/render/InventoryHud.h
    sdl/render/ObtainedBannerHud.h
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
