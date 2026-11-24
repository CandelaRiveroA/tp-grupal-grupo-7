#include "EditorWindow.h"

#include <QIcon>

#include <exception>
#include <filesystem>

#include "EditorDialog.h"
#include "NewWorldDialog.h"
#include "WelcomeView.h"
#include "WorkspaceView.h"
#include "qt_theme/GameCursor.h"
#include "world_model/ScenarioLibraryLoader.h"
#include "world_model/WorldFactory.h"

EditorWindow::EditorWindow(const std::string& scenariosDir,
			   const std::string& worldsDir, QWidget* parent)
	: QMainWindow(parent), worlds_(worldsDir) {
	if (std::filesystem::is_directory(scenariosDir)) {
		try {
			scenarios_ = worldmodel::ScenarioLibraryLoader()
					     .loadFromDirectory(scenariosDir);
		} catch (const std::exception& error) {
			EditorDialog(this).info(
				"Error al cargar escenarios",
				QString::fromStdString(error.what()));
		}
	}
	setWindowTitle("Taller Editor");
	setCursor(GameCursor().cursor());
	setWindowIcon(QIcon(":/editor/ui/editor_welcome_emblem.png"));

	pages_ = new QStackedWidget(this);
	setMinimumSize(800, 560);
	setCentralWidget(pages_);
	welcome_ = new WelcomeView(worlds_, this);
	workspace_ = new WorkspaceView(scenarios_, worlds_, this);
	pages_->addWidget(welcome_);
	pages_->addWidget(workspace_);

	connect(welcome_, &WelcomeView::openWorldRequested, this,
		&EditorWindow::openWorld);
	connect(welcome_, &WelcomeView::newWorldRequested, this,
		&EditorWindow::newWorld);
	connect(workspace_, &WorkspaceView::backRequested, this, [this] {
		pages_->setCurrentWidget(welcome_);
		welcome_->refresh();
	});
	welcome_->refresh();
}

void EditorWindow::openWorld(const std::string& name) {
	try {
		workspace_->setWorld(worlds_.load(name));
		pages_->setCurrentWidget(workspace_);
	} catch (const std::exception& error) {
		EditorDialog(this).info("Error al abrir el mundo",
					QString::fromStdString(error.what()));
	}
}

void EditorWindow::newWorld() {
	NewWorldDialog dialog(worlds_, this);
	if (dialog.exec() != QDialog::Accepted) {
		return;
	}
	try {
		const worldmodel::WorldFactory factory;
		workspace_->setWorld(factory.makeDefault(
			dialog.worldName(), scenarios_,
			factory.firstOfEachKind(scenarios_)));
		pages_->setCurrentWidget(workspace_);
	} catch (const std::exception& error) {
		EditorDialog(this).info(
			"No se pudo crear el mundo",
			QString::fromStdString(error.what()));
	}
}
