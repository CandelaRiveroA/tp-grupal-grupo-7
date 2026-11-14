#pragma once

#include <QString>

#include "world_model/Scenario.h"
#include "world_model/WorldVocabulary.h"

inline QString scenarioLabel(const worldmodel::Scenario& scenario) {
	return QString::fromStdString(
		scenario.id + " (" +
		worldmodel::WorldVocabulary().toText(scenario.kind) + ")");
}
