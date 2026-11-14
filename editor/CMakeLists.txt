# El editor solo depende de Qt6 y del modelo de mundos (sin SDL2).
# Se linkea desde este directorio: Qt6::Widgets es un target importado local
# y no hace falta promoverlo a global (el cliente ya lo hace en el suyo).
find_package(Qt6 COMPONENTS Widgets REQUIRED)

target_sources(taller_editor
    PRIVATE
    # .cpp files
    main.cpp
    EditorWindow.cpp
    WelcomeView.cpp
    WorkspaceView.cpp
    NewWorldDialog.cpp
    MapTreeWidget.cpp
    MapPlaceholderView.cpp
    ValidationPanel.cpp
    AddExtraMapDialog.cpp
    PaletteView.cpp
    PUBLIC
    # .h files (los headers con Q_OBJECT van acá: AUTOMOC los procesa)
    Constants.h
    EditorWindow.h
    WelcomeView.h
    WorkspaceView.h
    NewWorldDialog.h
    MapTreeWidget.h
    MapPlaceholderView.h
    ValidationPanel.h
    AddExtraMapDialog.h
    PaletteView.h
    )

target_link_libraries(taller_editor PRIVATE taller_common taller_world_model
                      Qt6::Widgets)

# Carpeta fija de presets de escenario y de mundos guardados.
target_compile_definitions(taller_editor PRIVATE
    TALLER_SCENARIOS_DIR=\"${CMAKE_SOURCE_DIR}/editor/scenarios\"
    TALLER_WORLDS_DIR=\"${CMAKE_SOURCE_DIR}/worlds\")

set_target_properties(taller_editor PROPERTIES AUTOMOC ON)
