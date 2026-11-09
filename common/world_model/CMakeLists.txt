target_sources(taller_world_model
    PRIVATE
    # .cpp files
    Geometry.cpp
    Tile.cpp
    TileCodec.cpp
    Map.cpp
    FormatError.cpp
    WorldVocabulary.cpp
    WorldTomlReader.cpp
    WorldSerializer.cpp
    PUBLIC
    # .h files
    Geometry.h
    Tile.h
    TileCodec.h
    MapObject.h
    Marker.h
    Portal.h
    Map.h
    World.h
    Scenario.h
    FormatError.h
    WorldVocabulary.h
    WorldTomlReader.h
    WorldSerializer.h
    ValidationIssue.h
    )
