# Harness de UI de Qt: compila los widgets reales del cliente y los ejercita
# offscreen (sin pantalla). No se compila por defecto:
#
#   cmake -S . -B build-harness -DTALLER_QT_HARNESS=ON
#   cmake --build build-harness --target qt_harness_screen qt_harness_sprites
#   QT_QPA_PLATFORM=offscreen ./build-harness/client/qt/harness/qt_harness_screen
#
# Cubre lo que taller_tests no puede: taller_tests no linkea Qt, asi que toda
# la capa de pantallas quedaria sin verificacion. Ver README.md.

find_package(Qt6 COMPONENTS Widgets REQUIRED)
set_target_properties(Qt6::Widgets PROPERTIES IMPORTED_GLOBAL TRUE)

set(HARNESS_QT_DIR ${CMAKE_SOURCE_DIR}/client/qt)

# Fuentes de Qt que necesitan los dos harness para instanciar la pantalla real.
set(HARNESS_QT_SOURCES
    ${HARNESS_QT_DIR}/resources/resources.qrc
    ${HARNESS_QT_DIR}/screens/CharacterScreen.cpp
    ${HARNESS_QT_DIR}/screens/Screen.cpp
    ${HARNESS_QT_DIR}/services/CharacterLayerSource.cpp
    ${HARNESS_QT_DIR}/services/SessionService.cpp
    ${HARNESS_QT_DIR}/services/CharacterSpriteComposer.cpp
    ${HARNESS_QT_DIR}/services/fakes/FakeSessionService.cpp
    ${HARNESS_QT_DIR}/theme/CharacterSheets.cpp
    ${HARNESS_QT_DIR}/theme/CharacterSprites.cpp
    ${HARNESS_QT_DIR}/theme/GameCursor.cpp
    ${HARNESS_QT_DIR}/theme/NineSlice.cpp
    ${HARNESS_QT_DIR}/theme/SkinRecolorer.cpp
    ${HARNESS_QT_DIR}/theme/TitleBackdrop.cpp
    ${HARNESS_QT_DIR}/theme/TitleSprites.cpp
    ${HARNESS_QT_DIR}/util/CharacterLayers.cpp
    ${HARNESS_QT_DIR}/util/IndexCycle.cpp
    ${HARNESS_QT_DIR}/widgets/CharacterPreview.cpp
    ${HARNESS_QT_DIR}/widgets/CycleRow.cpp
    ${HARNESS_QT_DIR}/widgets/CycleRow.h
    ${HARNESS_QT_DIR}/widgets/InputBox.cpp
    ${HARNESS_QT_DIR}/widgets/SliderRow.cpp
    ${HARNESS_QT_DIR}/widgets/SliderRow.h
    ${HARNESS_QT_DIR}/widgets/TitleButton.cpp)

add_executable(qt_harness_screen character_screen_check.cpp ${HARNESS_QT_SOURCES})
add_executable(qt_harness_sprites sprite_catalog_check.cpp ${HARNESS_QT_SOURCES}
               ${HARNESS_QT_DIR}/widgets/CharacterPreview.h)

foreach(harness qt_harness_screen qt_harness_sprites)
    set_target_properties(${harness} PROPERTIES AUTOMOC ON AUTORCC ON)
    target_include_directories(${harness} PRIVATE ${CMAKE_SOURCE_DIR}/client ${CMAKE_SOURCE_DIR})
    # La raiz del repo: los harness leen hojas y TOML desde el arbol de fuentes.
    target_compile_definitions(${harness} PRIVATE TALLER_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
    target_link_libraries(${harness} PRIVATE Qt6::Widgets taller_client_world)
endforeach()
