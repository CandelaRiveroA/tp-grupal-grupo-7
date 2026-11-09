target_sources(taller_tests
    PRIVATE
    # .cpp files
    character_layers.cpp
    index_cycle.cpp
    invite_code_format.cpp
    matches_monitor_test.cpp
    inventory_test.cpp
    world_test.cpp
    mineral_test.cpp
    world_interaction_mineral_test.cpp
    world_view_test.cpp
    animals_test.cpp
    furniture_test.cpp
    ../server/world_view.cpp
    ../server/matches_monitor.cpp
    ../server/match.cpp
    ../server/server_output_monitor.cpp
    ../server/commands/move_command.cpp
    ../server/commands/pick_command.cpp
    ../server/commands/take_command.cpp
    ../server/commands/sleep_command.cpp
    ../server/domain/inventory/item.cpp
    ../server/domain/inventory/product.cpp
    ../server/domain/inventory/decoration_item.cpp
    ../server/domain/inventory/slot.cpp
    ../server/domain/inventory/inventory.cpp
    ../server/domain/entities/player.cpp
    ../server/domain/entities/i_entity.cpp
    ../server/domain/entities/dropped_item.cpp
    ../server/domain/animals/animal.cpp
    ../server/domain/animals/chicken.cpp
    ../server/domain/animals/cow.cpp
    ../server/domain/furniture/furniture.cpp
    ../server/domain/furniture/bed.cpp
    ../server/domain/map/world.cpp
    ../server/domain/map/cell.cpp
    ../server/domain/minerals/mineral.cpp
    ../server/domain/minerals/coal_rock.cpp
    ../server/domain/minerals/lapis_rock.cpp
    ../server/domain/minerals/silver_rock.cpp
    ../server/domain/minerals/gold_rock.cpp    
    toml_document_test.cpp
    tile_ref_test.cpp
    tile_codec_test.cpp
    world_vocabulary_test.cpp
    world_format_error_test.cpp
    map_test.cpp
    world_toml_reader_test.cpp
    world_toml_writer_test.cpp
    world_serializer_test.cpp
    scene_config_loader_test.cpp
    scene_config_error_test.cpp
    audio_catalog_test.cpp
    sound_gate_test.cpp
    sound_cue_catalog_test.cpp
    character_catalog_loader_test.cpp
    character_palette_loader_test.cpp
    creature_data_test.cpp
    inventory_config_loader_test.cpp
    inventory_slots_test.cpp
    scene_pixel_mapper_test.cpp
    camera_test.cpp
    viewport_test.cpp
    element_layout_test.cpp
    enter_selection_test.cpp
    character_layout_test.cpp
    chunk_grid_test.cpp
    scene_element_index_test.cpp
    view_spatial_index_test.cpp
    world_contract_test.cpp
    mock_game_client_test.cpp
    mock_animal_simulation_test.cpp
    local_world_view_test.cpp
    frame_clock_test.cpp
    move_command_test.cpp
    mock_world_client_test.cpp
    floor_items_test.cpp
    content_ids_test.cpp
    action_names_test.cpp
    interaction_session_test.cpp
    object_picker_test.cpp
    cursor_config_loader_test.cpp
    dialog_config_loader_test.cpp
    dialog_animator_test.cpp
    dialog_layout_test.cpp
    dropped_item_layout_test.cpp
    player_controller_test.cpp
    walk_pacer_test.cpp
    view_motion_test.cpp
    footstep_cue_test.cpp
    resource_audio_cue_test.cpp
    resource_hit_smoke.cpp
    obtained_banner_config_loader_test.cpp
    obtained_banner_queue_test.cpp
    obtained_banner_layout_test.cpp
    action_resolver_test.cpp
    farm_catalogs_test.cpp
    farm_selection_test.cpp
    tool_use_sounds_test.cpp
    tool_overlay_loader_test.cpp
    tool_overlay_layout_test.cpp
    resource_config_loader_test.cpp
    digit_config_loader_test.cpp
    tile_layout_test.cpp
    entity_appearance_test.cpp
    startup_sequence_test.cpp
    logger.cpp
    # IndexCycle y CharacterLayers no tienen dependencias (ni Qt): el target
    # de tests los compila directo en vez de linkear el cliente.
    ${PROJECT_SOURCE_DIR}/client/qt/util/CharacterLayers.cpp
    ${PROJECT_SOURCE_DIR}/client/qt/util/IndexCycle.cpp
    PUBLIC
    # .h files
    )

# world_view.h incluye el dominio del servidor con rutas relativas a server/.
target_include_directories(taller_tests PRIVATE ${PROJECT_SOURCE_DIR}/server)

# InputManager consume SDL_Event/SDL_Keycode (headers de SDL2). SDL2 se
# fetchea solo si hay client/editor/tile-inspector (raíz del CMakeLists),
# así que los tests de input se agregan únicamente en esa configuración;
# en la config "solo tests" (make run-tests) no se compilan.
if(TALLER_CLIENT OR TALLER_EDITOR OR TALLER_TILE_INSPECTOR)
    target_sources(taller_tests
        PRIVATE
        input_manager_test.cpp
        # El target de tests lo compila directo (igual que CharacterLayers).
        ${PROJECT_SOURCE_DIR}/client/sdl/input/InputManager.cpp
    )
    # Linkear SDL2pp::SDL2pp propaga los includes de SDL2 (no se llaman
    # funciones SDL: solo se construyen eventos sintéticos).
    target_link_libraries(taller_tests SDL2pp::SDL2pp)
endif()
