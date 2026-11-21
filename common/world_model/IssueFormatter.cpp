#include "world_model/IssueFormatter.h"

#include <utility>

namespace worldmodel {

namespace {

// Fuente unica de verdad: cada IssueCode se describe una sola vez.
const std::pair<IssueCode, const char*> kIssueTexts[] = {
	{IssueCode::UnsupportedVersion, "version de formato no soportada"},
	{IssueCode::InvalidName, "nombre invalido"},
	{IssueCode::EmptyName, "el nombre del mundo no puede ser vacio"},
	{IssueCode::UnknownTileset, "tileset desconocido"},
	{IssueCode::MissingRequiredMap, "falta un mapa obligatorio"},
	{IssueCode::WrongMapKind, "el mapa no coincide con su slot"},
	{IssueCode::WrongDungeonLevel, "el nivel de mazmorra es incorrecto"},
	{IssueCode::InvalidMapId, "id de mapa invalido"},
	{IssueCode::InvalidDimensions, "dimensiones del mapa invalidas"},
	{IssueCode::GridSizeMismatch,
	 "la grilla no coincide con el tamano del mapa"},
	{IssueCode::CollisionOutOfSync, "la colision no coincide con el terreno"},
	{IssueCode::MissingObject, "falta un objeto requerido"},
	{IssueCode::DuplicateObject, "objeto duplicado"},
	{IssueCode::ObjectOutOfBounds, "objeto fuera de los limites"},
	{IssueCode::ObjectDoorMissing, "falta la puerta de un objeto"},
	{IssueCode::ObjectDoorOutOfBounds, "la puerta esta fuera de los limites"},
	{IssueCode::ObjectDoorNotWalkable, "la puerta no es transitable"},
	{IssueCode::MissingMarker, "falta un marcador requerido"},
	{IssueCode::DuplicateMarker, "marcador duplicado"},
	{IssueCode::UnexpectedMarker, "marcador no permitido en este mapa"},
	{IssueCode::MarkerOutOfBounds, "marcador fuera de los limites"},
	{IssueCode::MarkerNotWalkable,
	 "el marcador no esta sobre una celda transitable"},
	{IssueCode::MissingPortal, "falta un portal requerido"},
	{IssueCode::DuplicatePortal, "portal duplicado"},
	{IssueCode::UnexpectedPortal, "portal no permitido en este mapa"},
	{IssueCode::PortalOutOfBounds, "portal fuera de los limites"},
	{IssueCode::PortalNotWalkable,
	 "el portal no esta sobre una celda transitable"},
	{IssueCode::PortalTargetMissing, "el portal apunta a un mapa inexistente"},
	{IssueCode::PortalTargetOutOfBounds,
	 "el objetivo del portal esta fuera de los limites"},
	{IssueCode::PortalTargetNotWalkable,
	 "el objetivo del portal no es transitable"},
	{IssueCode::WrongPortalTarget, "el portal apunta al mapa equivocado"},
};

std::string descriptionOf(IssueCode code) {
	for (const auto& [candidate, text] : kIssueTexts) {
		if (candidate == code) {
			return text;
		}
	}
	return "problema desconocido";
}

}  // namespace

std::string IssueFormatter::format(const ValidationIssue& issue) const {
	std::string message = descriptionOf(issue.code);
	if (!issue.mapId.empty()) {
		message += " (mapa: " + issue.mapId + ")";
	}
	if (issue.cell.column >= 0 && issue.cell.row >= 0) {
		message += " (celda: " + std::to_string(issue.cell.column) +
			   ", " + std::to_string(issue.cell.row) + ")";
	}
	return message;
}

}  // namespace worldmodel
