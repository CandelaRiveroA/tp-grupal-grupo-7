#pragma once

#include <string>

#include "world_model/Geometry.h"

namespace worldmodel {

enum class IssueCode {
	UnsupportedVersion,
	InvalidName,
	EmptyName,
	UnknownTileset,
	MissingRequiredMap,
	WrongMapKind,
	WrongDungeonLevel,
	InvalidMapId,
	InvalidDimensions,
	GridSizeMismatch,
	CollisionOutOfSync,
	MissingObject,
	DuplicateObject,
	ObjectOutOfBounds,
	ObjectDoorMissing,
	ObjectDoorOutOfBounds,
	ObjectDoorNotWalkable,
	MissingMarker,
	DuplicateMarker,
	UnexpectedMarker,
	MarkerOutOfBounds,
	MarkerNotWalkable,
	MissingPortal,
	DuplicatePortal,
	UnexpectedPortal,
	PortalOutOfBounds,
	PortalNotWalkable,
	PortalTargetMissing,
	PortalTargetOutOfBounds,
	PortalTargetNotWalkable,
	WrongPortalTarget,
};

struct ValidationIssue {
	IssueCode code;
	std::string mapId;       // vacio para problemas del mundo
	TilePos cell{-1, -1};    // celda puntual, si aplica
};

}
