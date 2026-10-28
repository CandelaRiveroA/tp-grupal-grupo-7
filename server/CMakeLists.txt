target_sources(taller_server
    PRIVATE
    # .cpp files
    main.cpp
    server.cpp
    acceptor.cpp
    client_handler.cpp
    server_sender.cpp
    server_receiver.cpp
    match.cpp
    commands/move_command.cpp
    commands/pick_command.cpp
    commands/take_command.cpp
    server_output_monitor.cpp
    domain/map/cell.cpp
    domain/map/world.cpp
    domain/entities/i_entity.cpp
    domain/entities/player.cpp
    domain/inventory/inventory.cpp
    domain/inventory/item.cpp
    domain/inventory/product.cpp
    domain/inventory/slot.cpp
    PUBLIC
    # .h files
    server.h
    acceptor.h
    client_handler.h
    server_sender.h
    server_receiver.h
    match.h
    commands/commands.h
    server_output_monitor.h
    )

target_include_directories(taller_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
