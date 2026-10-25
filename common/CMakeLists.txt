target_sources(taller_common
    PRIVATE
    # .cpp files
    common_liberror.cpp
    common_resolver.cpp
    common_resolvererror.cpp
    common_socket.cpp
    common_protocol.cpp
    model/draft/ProvisionalInviteCodeFormat.cpp
    PUBLIC
    # .h files
    common_liberror.h
    common_resolver.h
    common_resolvererror.h
    common_socket.h
    common_protocol.h
    model/draft/CharacterAppearance.h
    model/draft/GameSelection.h
    model/draft/InviteCodeFormat.h
    model/draft/ProvisionalInviteCodeFormat.h
    model/draft/SaveSummary.h
    model/draft/WorldSummary.h
    )
