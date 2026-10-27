target_sources(taller_common
    PRIVATE
    # .cpp files
    common_liberror.cpp
    common_resolver.cpp
    common_resolvererror.cpp
    common_socket.cpp
    common_protocol_sender.cpp
    common_protocol_receiver.cpp
    common_worker.cpp
    PUBLIC
    # .h files
    common_liberror.h
    common_resolver.h
    common_resolvererror.h
    common_socket.h
    common_command_types.h
    common_command.h
    common_protocol_sender.h
    common_protocol_receiver.h
    common_server_status.h
    common_worker.h
    common_game_direction.h
    )
