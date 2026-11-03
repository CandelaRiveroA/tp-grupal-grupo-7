#pragma once

// Motivo por el que un servicio no pudo completar un pedido.
enum class ServiceError {
	Network,
	InvalidCode,
	GameFull,
	GameNameTaken,
	UsernameTaken,
	CharacterNameTaken,
	SaveNotFound,
};
