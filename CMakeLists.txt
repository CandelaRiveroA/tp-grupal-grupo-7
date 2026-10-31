cmake_minimum_required(VERSION 3.24)

if (NOT DEFINED CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER g++)
endif()

if (POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()

set(CMAKE_CXX_EXTENSIONS ON)

project(taller_tp
    VERSION 1.0
    HOMEPAGE_URL "https://github.com/CandelaRiveroA/tp-grupal-grupo-7"
    LANGUAGES CXX
    )

# Genera compile_commands.json en el build dir: lo usan IntelliSense de
# VS Code (cpptools) y clangd para resolver los include paths reales.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Export compile commands for IntelliSense/clangd")

option(TALLER_TESTS "Enable / disable tests." ON)
option(TALLER_CLIENT "Enable / disable client program." ON)
option(TALLER_SERVER "Enable / disable server program." ON)
option(TALLER_EDITOR "Enable / disable editor program." ON)
# Herramienta PROVISORIA para inspeccionar celdas de una hoja de sprites
# (ver client/sdl/dev/tile_inspector_main.cpp). OFF por defecto: no afecta
# el build normal del cliente/servidor/editor. Encenderla con
# -DTALLER_TILE_INSPECTOR=ON compila el ejecutable taller_tile_inspector.
option(TALLER_TILE_INSPECTOR "Enable / disable provisional tile inspector." OFF)
option(TALLER_MAKE_WARNINGS_AS_ERRORS "Enable / disable warnings as errors." ON)

message(CMAKE_CXX_COMPILER_ID="${CMAKE_CXX_COMPILER_ID}")

# Define target: lib common between client, server and editor programs
# HEY TODO XXX: change the name to something that you like
add_library(taller_common STATIC)

# add the source files that make the lib
# HEY!! TODO XXX: add more subfolders here
add_subdirectory(common/)


# set compiler warnings to the targets
include(cmake/CompilerWarnings.cmake)
set_project_warnings(taller_common ${TALLER_MAKE_WARNINGS_AS_ERRORS} FALSE)

# Say where the headers will be found required by library target
# Make this PUBLIC so any dependant linking lib will inherit this
# location (so our dependants will not have to explicitly request this)
# "." permite "#include \"common/foo.h\""; "common" permite el estilo corto
# "#include \"common_foo.h\"" que usan client/ y server/.
target_include_directories(taller_common PUBLIC . common)

# HEY!! TODO XXX: you need to install some runtime and dev libraries *before*
# compiling the client/editor code.
#
# In xubuntu 24.04 these are:
#
# sudo apt-get install libopus-dev libopusfile-dev libxmp-dev libfluidsynth-dev fluidsynth libwavpack1 libwavpack-dev libfreetype-dev wavpack
#

# Install libSDL2 and libSDL2pp (for client or editor)
# HEY!! TODO XXX: add more dependendies (either for client, server or editor)
if(TALLER_CLIENT OR TALLER_EDITOR OR TALLER_TILE_INSPECTOR)
    include(FetchContent)

    # libsdl2pp, al traerse vía FetchContent, no declara sus propias
    # options (solo lo hace cuando es el proyecto raíz): el padre tiene
    # que setearlas. SDL2PP_WITH_IMAGE habilita los constructores
    # Texture(renderer, path) / Surface(path) para cargar sprites desde
    # un archivo (PNG) sin recortar nada por frame en disco.
    set(SDL2PP_WITH_IMAGE ON)

    FetchContent_Declare(
        SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-2.30.8
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        OVERRIDE_FIND_PACKAGE
    )

    FetchContent_Declare(
        SDL2_image
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
        GIT_TAG release-2.8.2
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        OVERRIDE_FIND_PACKAGE
    )

    FetchContent_Declare(
        SDL2_mixer
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
        GIT_TAG release-2.8.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        OVERRIDE_FIND_PACKAGE
    )

    FetchContent_Declare(
        SDL2_ttf
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
        GIT_TAG release-2.22.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        OVERRIDE_FIND_PACKAGE
    )

    FetchContent_Declare(
        libSDL2pp
        OVERRIDE_FIND_PACKAGE
        URL https://github.com/libSDL2pp/libSDL2pp/archive/cc198c9a5657048bee67ece82de620b2d5661084.zip
    )

    FetchContent_MakeAvailable(SDL2 SDL2_image SDL2_mixer SDL2_ttf libSDL2pp)
endif()

# toml++: parser TOML header-only (C++17). Se usa para describir los
# escenarios como datos (ver client/world/scenes.toml) y cargarlos sin
# recompilar. Se trae con FetchContent igual que SDL2/GoogleTest; tag
# pinned (no rango abierto) para builds reproducibles. Lo necesitan el
# cliente (lo dibuja) y los tests (prueban el loader), así que se trae
# siempre que alguno de los dos esté habilitado.
if(TALLER_CLIENT OR TALLER_EDITOR OR TALLER_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        tomlplusplus
        GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
        GIT_TAG v3.4.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(tomlplusplus)
endif()

# Librería del cliente SIN dependencia de SDL: datos puros de la escena,
# el loader de TOML, el logger de la sección SDL (que solo escribe a
# stderr, no usa SDL), el FrameClock del game loop (cálculo puro de
# std::chrono, sin SDL ni sleeps), la geometría de TileLayout/ElementLayout,
# el Viewport y los índices espaciales (solo aritmética; el blit vive en
# ElementPainter). Al no depender de SDL, los tests la
# pueden linkear y probar esta lógica sin levantar una ventana. El
# cliente también la linkea. Se crea acá (raíz) porque la necesitan
# client y tests.
if(TALLER_CLIENT OR TALLER_TESTS)
    add_library(taller_client_world STATIC
        client/toml/TomlDocument.cpp
        client/world/ConfigError.cpp
        client/world/SceneConfigLoader.cpp
        client/world/SceneTomlReader.cpp
        client/world/CharacterCatalog.cpp
        client/world/CharacterCatalogLoader.cpp
        client/world/CreatureCatalog.cpp
        client/world/CharacterTomlReader.cpp
        client/world/CharacterLayout.cpp
        client/world/InventoryTomlReader.cpp
        client/world/InventoryConfigLoader.cpp
        client/world/InventoryLayout.cpp
        client/world/ItemCatalog.cpp
        client/world/ContentCatalog.cpp
        client/world/DigitCatalog.cpp
        client/world/DigitConfigLoader.cpp
        client/world/DigitTomlReader.cpp
        client/world/ToolOverlay.cpp
        client/world/ToolOverlayLayout.cpp
        client/world/ToolOverlayLoader.cpp
        client/world/ToolOverlayTomlReader.cpp
        client/world/ResourceCatalog.cpp
        client/world/ResourceConfigLoader.cpp
        client/world/ResourceTomlReader.cpp
        client/world/ObtainedBannerCatalog.cpp
        client/world/ObtainedBannerConfigLoader.cpp
        client/world/ObtainedBannerTomlReader.cpp
        client/world/ObtainedBannerLayout.cpp
        client/world/ScenePixelMapper.cpp
        client/world/ElementLayout.cpp
        client/world/ChunkGrid.cpp
        client/world/SceneElementIndex.cpp
        client/world/EntityAppearance.cpp
        client/game/ViewSpatialIndex.cpp
        client/game/FootstepCue.cpp
        client/game/ResourceAudioCue.cpp
        client/game/ObtainedBannerQueue.cpp
        client/game/MockGameClient.cpp
        client/game/MockAnimalSimulation.cpp
        client/game/SoundCue.cpp
        client/game/SoundGate.cpp
        client/game/MoveCommand.cpp
        client/game/PlayerController.cpp
        client/game/ViewMotion.cpp
        client/game/WalkPacer.cpp
        client/game/ActionResolver.cpp
        client/game/InventorySelector.cpp
        client/core/StartupSequence.cpp
        client/sdl/log/Logger.cpp
        client/sdl/loop/FrameClock.cpp
        client/sdl/render/Camera.cpp
        client/sdl/render/Viewport.cpp
        client/sdl/render/TileLayout.cpp)
    target_include_directories(taller_client_world PUBLIC client)
    target_link_libraries(taller_client_world PUBLIC tomlplusplus::tomlplusplus
                          taller_common)
    set_project_warnings(taller_client_world ${TALLER_MAKE_WARNINGS_AS_ERRORS} FALSE)
endif()

# Program section
# ---------------
if(TALLER_CLIENT)
    # HEY!! TODO XXX: Change the name of the executable to the one you like!
    add_executable(taller_client)

    # Make them depend on taller_common lib
    add_dependencies(taller_client taller_common SDL2pp::SDL2pp SDL2_mixer)

    # Add source files and enable warnings
    # HEY!! TODO XXX: add more subfolders here
    add_subdirectory(client)

    set_project_warnings(taller_client ${TALLER_MAKE_WARNINGS_AS_ERRORS} FALSE)

    # add libSDL2pp's headers
    target_include_directories(taller_client PRIVATE ${libSDL2pp_SOURCE_DIR})

    # Link the taller_common lib target
    # (keyword form: required so client/ can link imported targets
    # like Qt6::Widgets from its own directory scope)
    target_link_libraries(taller_client PRIVATE taller_common SDL2pp::SDL2pp
                          SDL2_mixer::SDL2_mixer taller_client_world)

    add_custom_target(client_assets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/client/sdl/assets
                ${CMAKE_BINARY_DIR}/assets
        COMMENT "Copiando assets del cliente al directorio del build")

    add_custom_target(client_sounds ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/client/sdl/sounds
                ${CMAKE_BINARY_DIR}/sounds
        COMMENT "Copiando sonidos del cliente al directorio del build")

    # Target de copia, no POST_BUILD: el POST_BUILD anterior solo corría
    # si el binario relinkeaba, y editar un TOML o sumar un .ogg dejaba
    # la copia stale. Un target ALL corre en TODA build, cambie el
    # código o no. (El cliente busca por TALLER_ASSET_DIR /
    # TALLER_SOUNDS_DIR, definidos en client/CMakeLists.txt.)
    add_dependencies(taller_client client_assets client_sounds)
endif()

# ------------------------------------------------------------
# Herramienta PROVISORIA: inspector de celdas de una hoja de sprites.
# Ejecutable aparte (no toca taller_client): su propio main() abre la
# ventana directo, sin lobby ni intro. Reusa SOLO las fuentes SDL que
# necesita (contexto, spritesheet, config de ventana) y nada de Qt.
# Borrar este bloque y client/sdl/dev/ lo elimina del proyecto.
# ------------------------------------------------------------
if(TALLER_TILE_INSPECTOR)
    add_executable(taller_tile_inspector
        client/sdl/dev/tile_inspector_main.cpp
        client/core/Constants.cpp
        client/core/WindowConfig.cpp
        client/sdl/context/SdlContext.cpp
        client/sdl/render/SpriteSheet.cpp)

    add_dependencies(taller_tile_inspector taller_common SDL2pp::SDL2pp)

    set_project_warnings(taller_tile_inspector
                         ${TALLER_MAKE_WARNINGS_AS_ERRORS} FALSE)

    # Headers del cliente ("core/...", "sdl/...") y de libSDL2pp.
    target_include_directories(taller_tile_inspector PRIVATE
        ${CMAKE_SOURCE_DIR}/client
        ${libSDL2pp_SOURCE_DIR})

    target_link_libraries(taller_tile_inspector PRIVATE
        taller_common SDL2pp::SDL2pp)

    # Mismo mecanismo de assets que el cliente: el inspector busca la
    # hoja por TALLER_ASSET_DIR y los assets se copian junto al binario.
    target_compile_definitions(taller_tile_inspector PRIVATE
        TALLER_ASSET_DIR=\"${CMAKE_BINARY_DIR}/assets\")

    if(TARGET client_assets)
        add_dependencies(taller_tile_inspector client_assets)
    else()
        add_custom_command(TARGET taller_tile_inspector POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_SOURCE_DIR}/client/sdl/assets
                    ${CMAKE_BINARY_DIR}/assets
            COMMENT "Copiando assets para el tile inspector")
    endif()
endif()

if(TALLER_SERVER)
    # HEY!! TODO XXX: Change the name of the executable to the one you like!
    add_executable(taller_server)

    # Make them depend on taller_common lib
    add_dependencies(taller_server taller_common)

    # Add source files and enable warnings
    # HEY!! TODO XXX: add more subfolders here
    add_subdirectory(server)

    set_project_warnings(taller_server ${TALLER_MAKE_WARNINGS_AS_ERRORS} FALSE)

    # Link the taller_common lib target
    target_link_libraries(taller_server taller_common)
endif()

if(TALLER_EDITOR)
    # HEY!! TODO XXX: Change the name of the executable to the one you like!
    add_executable(taller_editor)

    # Make them depend on taller_common lib
    add_dependencies(taller_editor taller_common SDL2pp::SDL2pp)

    # Add source files and enable warnings
    # HEY!! TODO XXX: add more subfolders here
    add_subdirectory(editor)

    set_project_warnings(taller_editor ${TALLER_MAKE_WARNINGS_AS_ERRORS} FALSE)

    # add libSDL2pp's headers
    target_include_directories(taller_editor PRIVATE ${libSDL2pp_SOURCE_DIR})

    # Link the taller_common lib target
    target_link_libraries(taller_editor taller_common SDL2pp::SDL2pp)
endif()


# Testing section
# ---------------

if(TALLER_TESTS)
    # Let's use google test
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )

    # For Windows: Prevent overriding the parent project's compiler/linker settings
    # (borrowed from https://google.github.io/googletest/quickstart-cmake.html)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    include(GoogleTest)

    # target for testing
    # HEY!! TODO XXX: Change the name of the executable to the one you like!
    add_executable(taller_tests)

    # HEY!! TODO XXX: I'm assuming that the code under testing is in taller_common
    # Change this if you have to.
    add_dependencies(taller_tests taller_common)

    # Say where to find the xoz headers
    # HEY!! TODO XXX: add more subfolders here
    target_include_directories(taller_tests PUBLIC .)

    # Add the source files to the taller_tests target
    # HEY!! TODO XXX: add more subfolders here
    add_subdirectory(tests/)

    # Set the compiler warnings to the tests target (but be a little lesser strict)
    set_project_warnings(taller_tests ${TALLER_MAKE_WARNINGS_AS_ERRORS} TRUE)

    # Say where googletest headers can be found
    target_include_directories(taller_tests
        PUBLIC
        ${gtest_SOURCE_DIR}/include
        ${gmock_SOURCE_DIR}/include
        )

    # HEY!! TODO XXX: I'm assuming that the code under testing is in taller_common
    # Change this if you have to.
    target_link_libraries(taller_tests
        taller_common
        taller_client_world
        GTest::gtest_main
    )

    # Ruta del scenes.toml real (en el árbol de fuentes) para el test de
    # integración del loader: así se verifica que el archivo que usa el
    # juego parsea y produce la escena esperada, sin copiarlo a mano.
    target_compile_definitions(taller_tests PRIVATE
        TALLER_SCENES_TOML=\"${CMAKE_SOURCE_DIR}/client/sdl/assets/scenes.toml\"
        TALLER_CHARACTERS_TOML=\"${CMAKE_SOURCE_DIR}/client/sdl/assets/characters.toml\"
        TALLER_INVENTORY_TOML=\"${CMAKE_SOURCE_DIR}/client/sdl/assets/inventory.toml\")
endif()
