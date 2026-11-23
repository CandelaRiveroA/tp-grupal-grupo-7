# El editor solo depende de Qt6 y del modelo de mundos (sin SDL2).
# Qt6::Widgets ya fue resuelto en el CMakeLists.txt de la raíz.

target_sources(taller_editor
    PRIVATE
    # .cpp files
    main.cpp
    EditorWindow.cpp
    WelcomeView.cpp
    WorkspaceView.cpp
    NewWorldDialog.cpp
    MapTreeWidget.cpp
    MapCanvasView.cpp
    MapLayerItem.cpp
    ValidationPanel.cpp
    AddExtraMapDialog.cpp
    PaletteView.cpp
    PortalsView.cpp
    TilesetCache.cpp
    MinimapGenerator.cpp
    WorldPainter.cpp
    PUBLIC
    # .h files (los headers con Q_OBJECT van acá: AUTOMOC los procesa)
    Constants.h
    EditorWindow.h
    WelcomeView.h
    WorkspaceView.h
    NewWorldDialog.h
    MapTreeWidget.h
    MapCanvasView.h
    MapLayerItem.h
    ValidationPanel.h
    AddExtraMapDialog.h
    PaletteView.h
    PortalsView.h
    TilesetCache.h
    MinimapGenerator.h
    WorldPainter.h
    )

target_link_libraries(taller_editor PRIVATE taller_common taller_world_model
                      Qt6::Widgets)

# Carpeta fija de presets de escenario, mundos guardados y assets.
target_compile_definitions(taller_editor PRIVATE
    TALLER_SCENARIOS_DIR=\"${CMAKE_SOURCE_DIR}/editor/scenarios\"
    TALLER_WORLDS_DIR=\"${CMAKE_SOURCE_DIR}/worlds\"
    TALLER_PREFABS_TOML=\"${CMAKE_SOURCE_DIR}/editor/prefabs.toml\"
    TALLER_ASSETS_DIR=\"${TALLER_ASSETS_DIR}\")

set_target_properties(taller_editor PROPERTIES AUTOMOC ON)
