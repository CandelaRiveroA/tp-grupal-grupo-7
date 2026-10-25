target_sources(taller_server
    PRIVATE
    # .cpp files
    main.cpp
    server_sender.cpp
    server_receiver.cpp
    match.cpp
    PUBLIC
    # .h files
    server_sender.h
    server_receiver.h
    match.h
    )
