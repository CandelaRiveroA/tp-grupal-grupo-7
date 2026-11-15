#pragma once

#include <string>

#include <QMainWindow>
#include <QStackedWidget>

#include "world_model/ScenarioLibrary.h"
#include "world_model/WorldRepository.h"

class WelcomeView;
class WorkspaceView;

// Pantalla principal del editor
class EditorWindow : public QMainWindow {
	Q_OBJECT
public:
	EditorWindow(const std::string& scenariosDir,
		     const std::string& worldsDir, QWidget* parent = nullptr);

private:
	void openWorld(const std::string& name);
	void newWorld();

	worldmodel::ScenarioLibrary scenarios_;
	worldmodel::WorldRepository worlds_;
	QStackedWidget* pages_ = nullptr;
	WelcomeView* welcome_ = nullptr;
	WorkspaceView* workspace_ = nullptr;
};
