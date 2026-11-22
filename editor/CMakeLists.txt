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
    TilesetCache.cpp
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
    TilesetCache.h
    )

target_link_libraries(taller_editor PRIVATE taller_common taller_world_model
                      Qt6::Widgets)

# Carpetas de presets de escenario, mundos guardados y assets. Se pueden
# sobreescribir al configurar (p. ej. para una instalacion en /opt/taller)
# pasando -DTALLER_INSTALL_*.
if(NOT DEFINED TALLER_INSTALL_SCENARIOS_DIR)
    set(TALLER_INSTALL_SCENARIOS_DIR "${CMAKE_SOURCE_DIR}/editor/scenarios")
endif()
if(NOT DEFINED TALLER_INSTALL_WORLDS_DIR)
    set(TALLER_INSTALL_WORLDS_DIR "${CMAKE_SOURCE_DIR}/worlds")
endif()
if(NOT DEFINED TALLER_INSTALL_PREFABS_TOML)
    set(TALLER_INSTALL_PREFABS_TOML "${CMAKE_SOURCE_DIR}/editor/prefabs.toml")
endif()
if(NOT DEFINED TALLER_INSTALL_ASSETS_DIR)
    set(TALLER_INSTALL_ASSETS_DIR "${TALLER_ASSETS_DIR}")
endif()
target_compile_definitions(taller_editor PRIVATE
    TALLER_SCENARIOS_DIR=\"${TALLER_INSTALL_SCENARIOS_DIR}\"
    TALLER_WORLDS_DIR=\"${TALLER_INSTALL_WORLDS_DIR}\"
    TALLER_PREFABS_TOML=\"${TALLER_INSTALL_PREFABS_TOML}\"
    TALLER_ASSETS_DIR=\"${TALLER_INSTALL_ASSETS_DIR}\")

set_target_properties(taller_editor PROPERTIES AUTOMOC ON)
