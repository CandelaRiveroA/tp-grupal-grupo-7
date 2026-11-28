# Qt6::Widgets se resuelve una única vez en el CMakeLists.txt de la raíz
# para que sea visible tanto en el scope raíz (AUTOMOC/AUTORCC) como en
# todos los subdirectorios (client, editor, harness) sin conflictos de export set.

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
    ${CMAKE_SOURCE_DIR}/common/qt_theme/resources/qt_theme.qrc
    qt/screens/CharacterScreen.cpp
    qt/screens/JoinCodeScreen.cpp
    qt/screens/MainMenuScreen.cpp
    qt/screens/SaveListScreen.cpp
    qt/screens/Screen.cpp
    qt/screens/UsernameScreen.cpp
    qt/services/CharacterLayerSource.cpp
    qt/services/SaveService.cpp
    qt/services/SessionService.cpp
    qt/services/CharacterSpriteComposer.cpp
    qt/services/NetworkSessionService.cpp
    qt/services/NetworkWorldService.cpp
    qt/services/WorldService.cpp
    qt/services/fakes/FakeSaveService.cpp
    qt/theme/CharacterSheets.cpp
    qt/theme/TitleSprites.cpp
    qt/util/CharacterLayers.cpp
    qt/util/IndexCycle.cpp
    qt/widgets/AsyncStateView.cpp
    qt/widgets/CharacterAppearanceEditor.cpp
    qt/widgets/CharacterPreview.cpp
    qt/widgets/CycleRow.cpp
    qt/widgets/SaveRowButton.cpp
    qt/widgets/WorldButton.cpp
    qt/widgets/UsernamePlate.cpp
    qt/widgets/WorldSelectorWidget.cpp
    qt/theme/CharacterSprites.cpp
    qt/theme/JoinCodeSprites.cpp
    qt/theme/SkinRecolorer.cpp
    qt/theme/SaveListSprites.cpp
    qt/theme/WorldSprites.cpp
    qt/widgets/InputBox.cpp
    qt/widgets/SliderRow.cpp
    qt/widgets/ParchmentBanner.cpp
    qt/widgets/PromptForm.cpp
    world/Scenario.cpp
    sdl/context/SdlContext.cpp
    sdl/loop/GameLoop.cpp
    sdl/render/Renderer.cpp
    sdl/render/ElementPainter.cpp
    sdl/render/TileLayerPainter.cpp
    sdl/render/EntityRenderer.cpp
    sdl/render/SheetCache.cpp
    sdl/render/SpriteSheet.cpp
    sdl/render/CharacterSprite.cpp
    sdl/render/PixelComposer.cpp
    sdl/render/PlayerAppearancePainter.cpp
    sdl/render/ToolOverlayPainter.cpp
    sdl/render/HeldItemPainter.cpp
    sdl/render/GroundEffectPainter.cpp
    sdl/render/CropPainter.cpp
    sdl/render/BuildPlacePainter.cpp
    sdl/render/FurnitureGhostPainter.cpp
    sdl/render/PropPainter.cpp
    sdl/render/InventoryHud.cpp
    sdl/render/ObtainedBannerHud.cpp
    sdl/render/NoticePainter.cpp
    sdl/render/NameplatePainter.cpp
    sdl/render/HudOverlayPainter.cpp
    sdl/render/CursorPainter.cpp
    sdl/render/DialogPainter.cpp
    sdl/render/TextRenderer.cpp
    sdl/render/PromptPainter.cpp
    sdl/render/ButtonPainter.cpp
    sdl/render/DigitPainter.cpp
    sdl/render/SettingsPainter.cpp
    sdl/render/NpcRenderer.cpp
    sdl/render/NpcPopupPainter.cpp
    sdl/render/CloseButtonPainter.cpp
    sdl/render/MarketPopupPainter.cpp
    sdl/render/CommunityPopupPainter.cpp
    sdl/render/PagedModalPainter.cpp
    sdl/render/WorldMapPainter.cpp
    sdl/render/ChatPainter.cpp
    sdl/input/InputManager.cpp
    sdl/audio/AudioPlayer.cpp
    sdl/audio/AudioMixer.cpp
    sdl/audio/MusicPlayer.cpp
    PUBLIC
    # .h files (los headers con Q_OBJECT van acá: AUTOMOC los necesita)
    core/Application.h
    core/Constants.h
    core/MusicControl.h
    core/MusicTrack.h
    core/SessionConfig.h
    core/PlaySession.h
    core/FarmConstants.h
    core/ToolUseSounds.h
    core/ActionSounds.h
    core/StartupSequence.h
    core/StartupScreens.h
    core/WindowConfig.h
    qt/LobbyWindow.h
    qt/screens/CharacterScreen.h
    qt/screens/JoinCodeScreen.h
    qt/screens/MainMenuScreen.h
    qt/screens/SaveListScreen.h
    qt/screens/Screen.h
    qt/screens/UsernameScreen.h
    qt/services/CharacterLayerSource.h
    qt/services/CharacterSpriteComposer.h
    qt/services/SaveService.h
    qt/services/ServiceError.h
    qt/services/SessionService.h
    qt/services/NetworkSessionService.h
    qt/services/NetworkWorldService.h
    qt/services/WorldService.h
    qt/services/fakes/FakeLatency.h
    qt/services/fakes/FakeSaveService.h
    qt/theme/CharacterRects.h
    qt/theme/JoinCodeRects.h
    qt/theme/CharacterSprites.h
    qt/theme/JoinCodeSprites.h
    qt/theme/SaveListRects.h
    qt/theme/SaveListSprites.h
    qt/theme/WorldSpriteRects.h
    qt/theme/WorldSprites.h
    qt/theme/CharacterSheetRects.h
    qt/theme/CharacterSheets.h
    qt/theme/SkinRecolorer.h
    qt/theme/TitleSprites.h
    qt/theme/UsernameRects.h
    qt/util/CharacterLayer.h
    qt/util/CharacterLayers.h
    qt/util/IndexCycle.h
    qt/widgets/AsyncStateView.h
    qt/widgets/CharacterAppearanceEditor.h
    qt/widgets/CharacterPreview.h
    qt/widgets/CycleRow.h
    qt/widgets/InputBox.h
    qt/widgets/SaveRowButton.h
    qt/widgets/WorldButton.h
    qt/widgets/UsernamePlate.h
    qt/widgets/WorldSelectorWidget.h
    qt/widgets/SliderRow.h
    qt/widgets/ParchmentBanner.h
    qt/widgets/PromptForm.h
    client_sender.h
    client_receiver.h
    game/FootstepCue.h
    game/ResourceAudioCue.h
    game/ChatCue.h
    game/ObtainedBannerQueue.h
    game/NoticeQueue.h
    world/NoticeLayout.h
    world/SettingsLayout.h
    world/SliderLayout.h
    world/SoundCategory.h
    game/GameClient.h
    game/GameConstants.h
    game/InputCommand.h
    game/FractionStat.h
    game/LocalWorldView.h
    game/NetworkGameClient.h
    game/MoveCommand.h
    game/Interactor.h
    game/FloorItems.h
    game/InteractionSession.h
    game/CropSystem.h
    game/DialogAnimator.h
    game/PlayerController.h
    game/PointerClick.h
    game/ObjectPicker.h
    game/ViewMotion.h
    game/WalkPacer.h
    game/ActionResolver.h
    game/NpcPlacementResolver.h
    game/MarketPopupState.h
    game/SettingsState.h
    game/WorldMapState.h
    game/MarketSetCycle.h
    game/PropAnimator.h
    game/SlotStacker.h
    game/ContainerSession.h
    game/ContainerTransfer.h
    game/FurnitureGrab.h
    game/InventoryDiscard.h
    game/FurniturePlacement.h
    game/FurnitureKey.h
    sdl/render/PropPainter.h
    game/PlacementSelection.h
    world/FurnitureFootprint.h
    world/FurnitureGeometry.h
    world/FurnitureViews.h
    world/FurnitureFacing.h
    game/FurnitureOrientation.h
    game/InventorySelector.h
    world/ActionNames.h
    world/CharacterAppearance.h
    world/CharacterCatalog.h
    world/CharacterCatalogLoader.h
    world/CharacterLayout.h
    world/WorldConstants.h
    world/CharacterTomlReader.h
    world/Color.h
    world/ConfigError.h
    world/ContentIds.h
    world/CreatureCatalog.h
    world/EntityAppearance.h
    world/Grid.h
    world/Scenario.h
    world/InventoryLayout.h
    world/InventoryTomlReader.h
    world/InventoryConfigLoader.h
    world/ItemKind.h
    world/SpriteVisual.h
    world/CursorCatalog.h
    world/CursorConfigLoader.h
    world/CursorTomlReader.h
    world/Indication.h
    world/DialogCatalog.h
    world/DialogConfigLoader.h
    world/DialogTomlReader.h
    world/DialogLayout.h
    world/DroppedItemLayout.h
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
    world/PixelBox.h
    world/TomlConfigReader.h
    world/CreatureKindNames.h
    world/WorldMapConfig.h
    world/WorldMapLayout.h
    world/PagedModal.h
    world/AnchorLayout.h
    world/HudOverlayCatalog.h
    world/HudOverlayTomlReader.h
    world/HudFillLayout.h
    world/HudOverlayConfigLoader.h
    world/SceneDescription.h
    world/SceneKeys.h
    world/ScenePixelMapper.h
    world/SceneTomlReader.h
    world/ElementLayout.h
    world/ChunkGrid.h
    world/SceneElementIndex.h
    game/ViewSpatialIndex.h
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
    sdl/render/TileLayerPainter.h
    sdl/render/Viewport.h
    sdl/render/Camera.h
    sdl/render/CharacterSprite.h
    sdl/render/PixelComposer.h
    sdl/render/PlayerAppearancePainter.h
    sdl/render/RenderConstants.h
    sdl/render/ToolOverlayPainter.h
    sdl/render/HeldItemPainter.h
    sdl/render/GroundEffectPainter.h
    sdl/render/CropPainter.h
    sdl/render/BuildPlacePainter.h
    sdl/render/FurnitureGhostPainter.h
    sdl/render/InventoryHud.h
    sdl/render/ObtainedBannerHud.h
    sdl/render/NoticePainter.h
    sdl/render/NameplatePainter.h
    world/NameplateLayout.h
    sdl/render/HudOverlayPainter.h
    sdl/render/CursorPainter.h
    sdl/render/DialogPainter.h
    sdl/render/TextRenderer.h
    sdl/render/PromptPainter.h
    sdl/render/ButtonPainter.h
    sdl/render/DigitPainter.h
    sdl/render/SettingsPainter.h
    sdl/render/PagedModalPainter.h
    sdl/render/WorldMapPainter.h
    sdl/render/ChatPainter.h
    sdl/input/InputManager.h
    sdl/audio/AudioCatalog.h
    sdl/audio/AudioConfig.h
    sdl/audio/AudioPlayer.h
    sdl/audio/AudioMixer.h
    sdl/audio/MusicPlayer.h
    sdl/log/Logger.h
    )

# AUTOMOC y AUTORCC solo para este target (no afectan a server ni a tests)
set_target_properties(taller_client PROPERTIES AUTOMOC ON AUTORCC ON)

# La copia de assets al build (add_custom_command POST_BUILD) vive en la
# raíz: el target taller_client se crea ahí y CMake solo permite esa
# forma del comando en el directorio donde se creó el target.
if(NOT DEFINED TALLER_INSTALL_ASSETS_DIR)
    set(TALLER_INSTALL_ASSETS_DIR "${CMAKE_BINARY_DIR}/assets")
endif()

if(NOT DEFINED TALLER_INSTALL_SOUNDS_DIR)
    set(TALLER_INSTALL_SOUNDS_DIR "${CMAKE_BINARY_DIR}/sounds")
endif()

if(NOT DEFINED TALLER_INSTALL_WORLDS_DIR)
    set(TALLER_INSTALL_WORLDS_DIR "${CMAKE_SOURCE_DIR}/worlds")
endif()

target_compile_definitions(taller_client PRIVATE
    "TALLER_ASSET_DIR=\"${TALLER_INSTALL_ASSETS_DIR}\""
    "TALLER_SOUNDS_DIR=\"${TALLER_INSTALL_SOUNDS_DIR}\""
    "TALLER_WORLDS_DIR=\"${TALLER_INSTALL_WORLDS_DIR}\""
)

# La raíz solo expone el repo root como include ("common/foo.h");
# con esto los includes del cliente son "core/...", "qt/...", etc.
target_include_directories(taller_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Qt6::Widgets es un target importado definido en este subdir (por el
# find_package de arriba): hay que linkearlo con forma keyword (PRIVATE)
# para que CMake lo resuelva desde acá y no desde la raíz.
target_link_libraries(taller_client PRIVATE Qt6::Widgets taller_qt_theme)

