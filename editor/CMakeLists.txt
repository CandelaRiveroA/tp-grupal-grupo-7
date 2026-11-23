# El editor solo depende de Qt6 y del modelo de mundos (sin SDL2).
# Qt6::Widgets ya fue resuelto en el CMakeLists.txt de la raíz.

set(EDITOR_SOURCES
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
    NineSliceFrame.cpp
    EditorDialog.cpp
    resources/editor_resources.qrc
    ${CMAKE_SOURCE_DIR}/common/qt_theme/resources/qt_theme.qrc
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
    NineSliceFrame.h
    EditorDialog.h
    )

target_sources(taller_editor
    PRIVATE
    main.cpp
    ${EDITOR_SOURCES}
    )

target_link_libraries(taller_editor PRIVATE taller_common taller_world_model
                      taller_qt_theme Qt6::Widgets)

# Carpeta fija de presets de escenario, mundos guardados y assets.
target_compile_definitions(taller_editor PRIVATE
    TALLER_SCENARIOS_DIR=\"${CMAKE_SOURCE_DIR}/editor/scenarios\"
    TALLER_WORLDS_DIR=\"${CMAKE_SOURCE_DIR}/worlds\"
    TALLER_PREFABS_TOML=\"${CMAKE_SOURCE_DIR}/editor/prefabs.toml\"
    TALLER_ASSETS_DIR=\"${TALLER_ASSETS_DIR}\")

set_target_properties(taller_editor PROPERTIES AUTOMOC ON AUTORCC ON)

if(TALLER_QT_HARNESS)
    add_executable(qt_harness_editor
        harness/editor_capture_check.cpp
        ${EDITOR_SOURCES})
    target_include_directories(qt_harness_editor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(qt_harness_editor PRIVATE taller_common taller_world_model
                          taller_qt_theme Qt6::Widgets)
    target_compile_definitions(qt_harness_editor PRIVATE
        TALLER_SCENARIOS_DIR=\"${CMAKE_SOURCE_DIR}/editor/scenarios\"
        TALLER_WORLDS_DIR=\"${CMAKE_SOURCE_DIR}/worlds\"
        TALLER_PREFABS_TOML=\"${CMAKE_SOURCE_DIR}/editor/prefabs.toml\"
        TALLER_ASSETS_DIR=\"${TALLER_ASSETS_DIR}\"
        TALLER_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\")
    set_target_properties(qt_harness_editor PROPERTIES AUTOMOC ON AUTORCC ON)
    set_project_warnings(qt_harness_editor ${TALLER_MAKE_WARNINGS_AS_ERRORS} FALSE)
endif()
