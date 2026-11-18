#include "WorkspaceView.h"

#include <exception>
#include <vector>

#include <QButtonGroup>
#include <QDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QStringList>
#include <QToolButton>
#include <QVBoxLayout>

#include "AddExtraMapDialog.h"
#include "Constants.h"
#include "MapCanvasView.h"
#include "MapTreeWidget.h"
#include "PaletteView.h"
#include "ScenarioLabel.h"
#include "ValidationPanel.h"
#include "world_model/FormatError.h"
#include "world_model/PrefabCatalogLoader.h"
#include "world_model/WorldFactory.h"
#include "world_model/WorldValidator.h"

WorkspaceView::WorkspaceView(const worldmodel::ScenarioLibrary& scenarios,
			     const worldmodel::WorldRepository& worlds,
			     QWidget* parent)
	: QWidget(parent),
	  scenarios_(scenarios),
	  worlds_(worlds),
	  tilesets_(editor::kAssetsDir) {
	try {
		catalog_ = worldmodel::PrefabCatalogLoader().loadFromFile(
			editor::kPrefabsTomlPath);
	} catch (const std::exception& error) {
		QMessageBox::warning(this, "No se pudo cargar prefabs",
				     QString::fromStdString(error.what()));
	}
	tilesets_.loadTilesets(catalog_.tilesets());

	tree_ = new MapTreeWidget(this);
	canvas_ = new MapCanvasView(tilesets_, this);
	palette_ = new PaletteView(catalog_, tilesets_, this);

	// La validacion vive en un dialogio
	validationDialog_ = new QDialog(this);
	validationDialog_->setWindowTitle("Validacion del mundo");
	validationPanel_ = new ValidationPanel(validationDialog_);
	QVBoxLayout* dialogLayout = new QVBoxLayout(validationDialog_);
	dialogLayout->addWidget(validationPanel_);
	validationDialog_->resize(editor::kValidationDialogSize);

	QSplitter* bodySplit = new QSplitter(Qt::Horizontal, this);
	bodySplit->addWidget(tree_);
	bodySplit->addWidget(canvas_);
	bodySplit->addWidget(palette_);
	bodySplit->setStretchFactor(0, 0);
	bodySplit->setStretchFactor(1, 1);
	bodySplit->setStretchFactor(2, 0);
	bodySplit->setSizes(editor::kBodySplitterSizes);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addLayout(buildTopBar());
	layout->addWidget(bodySplit);

	connect(tree_, &MapTreeWidget::mapSelected, this,
		&WorkspaceView::selectMap);
	connect(tree_, &MapTreeWidget::addExtraMapRequested, this,
		&WorkspaceView::addExtraMap);
	connect(tree_, &MapTreeWidget::removeExtraMapRequested, this,
		&WorkspaceView::removeExtraMap);
	connect(tree_, &MapTreeWidget::changeScenarioRequested, this,
		&WorkspaceView::replaceScenario);
	connect(validationPanel_, &ValidationPanel::mapSelected, this,
		[this](const std::string& mapId) {
			// Problemas del mundo (mapId vacio) o de un mapa que falta.
			if (world_.maps.contains(mapId)) {
				selectMap(mapId);
			}
		});
	connect(palette_, &PaletteView::prefabSelected, this,
		&WorkspaceView::onPrefabSelected);
	connect(canvas_, &MapCanvasView::mapEdited, this, [this] {
		if (placeButton_->isChecked() && activePrefab_ != nullptr) {
			const std::string& tilesetId =
				activePrefab_->tile.tileset;
			const auto catIt = catalog_.tilesets().find(tilesetId);
			if (catIt != catalog_.tilesets().end() &&
			    !world_.tilesets.contains(tilesetId)) {
				world_.tilesets.emplace(tilesetId,
							catIt->second);
			}
		}
		setDirty();
	});
}

QHBoxLayout* WorkspaceView::buildTopBar() {
	heading_ = new QLabel(this);
	heading_->setIndent(editor::kHeadingIndent);
	QPushButton* backButton = new QPushButton("Volver al inicio", this);
	QPushButton* validateButton = new QPushButton("Validar", this);
	QPushButton* saveButton = new QPushButton("Guardar", this);
	QToolButton* mapsButton = new QToolButton(this);
	mapsButton->setText("Mapas");
	mapsButton->setCheckable(true);
	mapsButton->setChecked(true);

	QToolButton* navigateButton = new QToolButton(this);
	navigateButton->setText("Navegar");
	navigateButton->setCheckable(true);
	navigateButton->setChecked(true);
	placeButton_ = new QToolButton(this);
	placeButton_->setText("Colocar");
	placeButton_->setCheckable(true);
	QToolButton* eraseButton = new QToolButton(this);
	eraseButton->setText("Borrar");
	eraseButton->setCheckable(true);

	QButtonGroup* toolGroup = new QButtonGroup(this);
	toolGroup->setExclusive(true);
	toolGroup->addButton(navigateButton, static_cast<int>(EditorTool::Navigate));
	toolGroup->addButton(placeButton_, static_cast<int>(EditorTool::Place));
	toolGroup->addButton(eraseButton, static_cast<int>(EditorTool::Erase));

	QToolButton* collisionButton = new QToolButton(this);
	collisionButton->setText("Colision");
	collisionButton->setCheckable(true);
	QToolButton* zoomOutButton = new QToolButton(this);
	zoomOutButton->setText("-");
	QToolButton* zoomResetButton = new QToolButton(this);
	zoomResetButton->setText("100%");
	QToolButton* zoomInButton = new QToolButton(this);
	zoomInButton->setText("+");

	QHBoxLayout* top = new QHBoxLayout;
	top->addWidget(mapsButton);
	top->addWidget(heading_);
	top->addStretch();
	top->addWidget(navigateButton);
	top->addWidget(placeButton_);
	top->addWidget(eraseButton);
	top->addSpacing(editor::kTopBarSpacing);
	top->addWidget(collisionButton);
	top->addSpacing(editor::kTopBarSpacing);
	top->addWidget(zoomOutButton);
	top->addWidget(zoomResetButton);
	top->addWidget(zoomInButton);
	top->addSpacing(editor::kTopBarSpacing);
	top->addWidget(backButton);
	top->addWidget(validateButton);
	top->addWidget(saveButton);

	connect(mapsButton, &QToolButton::toggled, tree_, &QWidget::setVisible);
	connect(toolGroup, &QButtonGroup::idToggled, this,
		[this](int id, bool checked) {
			if (checked) {
				canvas_->setTool(static_cast<EditorTool>(id));
			}
		});
	connect(collisionButton, &QToolButton::toggled, canvas_,
		&MapCanvasView::setShowCollision);
	connect(zoomOutButton, &QToolButton::clicked, canvas_,
		&MapCanvasView::zoomOut);
	connect(zoomResetButton, &QToolButton::clicked, canvas_,
		&MapCanvasView::resetZoom);
	connect(zoomInButton, &QToolButton::clicked, canvas_,
		&MapCanvasView::zoomIn);
	connect(backButton, &QPushButton::clicked, this, [this] {
		if (dirty_) {
			const QMessageBox::StandardButton answer =
				QMessageBox::question(
					this, "Cambios sin guardar",
					"Hay cambios sin guardar. Los "
					"descarto y vuelvo al inicio?");
			if (answer != QMessageBox::Yes) {
				return;
			}
		}
		emit backRequested();
	});
	connect(validateButton, &QPushButton::clicked, this,
		&WorkspaceView::validateNow);
	connect(saveButton, &QPushButton::clicked, this,
		&WorkspaceView::saveNow);
	return top;
}

void WorkspaceView::setWorld(const worldmodel::World& world) {
	canvas_->setMap(nullptr);
	world_ = world;
	dirty_ = false;
	palette_->clearSelection();
	setActivePrefab(nullptr);
	tilesets_.loadTilesets(catalog_.tilesets());
	tilesets_.loadTilesets(world_.tilesets);
	validationPanel_->showIssues({});
	updateHeading();
	tree_->setExtraMaps(world_);
	selectMap(editor::kHomeMapId);
}

void WorkspaceView::selectMap(const std::string& mapId) {
	activeMapId_ = mapId;
	tree_->selectMap(mapId);
	canvas_->setMap(world_.maps.contains(mapId) ? &world_.maps.at(mapId)
						    : nullptr);
}

void WorkspaceView::validateNow() {
	validationPanel_->showIssues(
		worldmodel::WorldValidator().validate(world_));
	validationDialog_->show();
	validationDialog_->raise();
}

void WorkspaceView::saveNow() {
	std::vector<worldmodel::ValidationIssue> issues;
	try {
		issues = worlds_.saveIfValid(world_);
	} catch (const std::exception& error) {
		QMessageBox::critical(this, "No se pudo guardar",
				      QString::fromStdString(error.what()));
		return;
	}
	validationPanel_->showIssues(issues);
	if (!issues.empty()) {
		QMessageBox::warning(
			this, "Guardar bloqueado",
			"No se puede guardar: el validador reporto " +
				QString::number(issues.size()) +
				" problema(s). Corregilos antes de guardar.");
		return;
	}
	dirty_ = false;
	updateHeading();
	QMessageBox::information(
		this, "Mundo guardado",
		QString::fromStdString("Guardado en " +
				       worlds_.filePathFor(world_.name)));
}

void WorkspaceView::addExtraMap() {
	AddExtraMapDialog dialog(scenarios_, world_, this);
	if (dialog.exec() != QDialog::Accepted) {
		return;
	}
	try {
		worldmodel::WorldFactory().addExtraMap(world_, *dialog.scenario(),
						       dialog.mapId());
	} catch (const worldmodel::FormatError& error) {
		QMessageBox::critical(
			this, "No se pudo agregar el mapa",
			QString::fromStdString(error.what()));
		return;
	}
	tilesets_.loadTilesets(world_.tilesets);
	setDirty();
	tree_->setExtraMaps(world_);
	selectMap(dialog.mapId());
}

void WorkspaceView::removeExtraMap() {
	const QMessageBox::StandardButton answer = QMessageBox::question(
		this, "Eliminar mapa extra",
		QString::fromStdString("Elimino el mapa " + activeMapId_ +
				       "? No hay deshacer."));
	if (answer != QMessageBox::Yes) {
		return;
	}
	canvas_->setMap(nullptr);
	worldmodel::WorldFactory().removeExtraMap(world_, activeMapId_);
	setDirty();
	tree_->setExtraMaps(world_);
	selectMap(editor::kHomeMapId);
}

void WorkspaceView::replaceScenario() {
	const bool mandatory = rules_.find(activeMapId_) != nullptr;
	const std::vector<const worldmodel::Scenario*> options =
		scenarioOptions();
	if (options.empty()) {
		QMessageBox::information(this, "Cambiar escenario base",
					 "No hay escenarios disponibles.");
		return;
	}

	QStringList labels;
	for (const worldmodel::Scenario* scenario : options) {
		labels << scenarioLabel(*scenario);
	}
	bool ok = false;
	const QString chosen =
		QInputDialog::getItem(this, "Cambiar escenario base",
				      "Nuevo escenario:", labels, 0, false, &ok);
	if (!ok) {
		return;
	}
	const worldmodel::Scenario& scenario = *options[labels.indexOf(chosen)];

	const QMessageBox::StandardButton answer = QMessageBox::question(
		this, "Cambiar escenario base",
		QString::fromStdString(
			"Reemplazar el escenario base de " + activeMapId_ +
			" por " + scenario.id +
			"? Se reinicia el terreno y la colision del mapa" +
			(mandatory
			 ? " y se recolocan sus puntos por defecto"
			 : "") +
			"."),
		QMessageBox::Yes | QMessageBox::No);
	if (answer != QMessageBox::Yes) {
		return;
	}
	canvas_->setMap(nullptr);
	try {
		worldmodel::WorldFactory().replaceMapScenario(
			world_, activeMapId_, scenario);
	} catch (const worldmodel::FormatError& error) {
		// replaceMapScenario opera sobre copia, asi que world_ queda intacta.
		if (world_.maps.contains(activeMapId_)) {
			canvas_->setMap(&world_.maps.at(activeMapId_));
		}
		QMessageBox::critical(
			this, "No se pudo reemplazar",
			QString::fromStdString(error.what()));
		return;
	}
	tilesets_.loadTilesets(catalog_.tilesets());
	tilesets_.loadTilesets(world_.tilesets);
	setDirty();
	canvas_->setMap(&world_.maps.at(activeMapId_));
}

// Obligatorio: solo escenarios del tipo del slot. Extra: todos.
std::vector<const worldmodel::Scenario*> WorkspaceView::scenarioOptions()
	const {
	if (rules_.find(activeMapId_) != nullptr) {
		return scenarios_.byKind(world_.maps.at(activeMapId_).kind);
	}
	std::vector<const worldmodel::Scenario*> all;
	for (const worldmodel::Scenario& scenario : scenarios_.all()) {
		all.push_back(&scenario);
	}
	return all;
}

void WorkspaceView::onPrefabSelected(const worldmodel::PrefabDef* prefab) {
	if (prefab == nullptr) {
		setActivePrefab(nullptr);
		return;
	}
	if (hasTilesetConflict(prefab->tile.tileset)) {
		QMessageBox::warning(
			this, "Conflicto de tileset",
			QString::fromStdString(
				"El tileset '" + prefab->tile.tileset +
				"' del mundo no coincide con "
				"el catalogo de prefabs."));
		palette_->clearSelection();
		setActivePrefab(nullptr);
		return;
	}
	setActivePrefab(prefab);
	placeButton_->setChecked(true);
	canvas_->setTool(EditorTool::Place);
}

void WorkspaceView::setActivePrefab(const worldmodel::PrefabDef* prefab) {
	activePrefab_ = prefab;
	canvas_->setActivePrefab(prefab);
}

void WorkspaceView::setDirty() {
	dirty_ = true;
	updateHeading();
}

void WorkspaceView::updateHeading() {
	heading_->setText(QString::fromStdString(world_.name +
						 (dirty_ ? " *" : "")));
}

bool WorkspaceView::hasTilesetConflict(const std::string& tilesetId) const {
	const auto worldIt = world_.tilesets.find(tilesetId);
	if (worldIt == world_.tilesets.end()) {
		return false;
	}
	const auto catIt = catalog_.tilesets().find(tilesetId);
	return catIt == catalog_.tilesets().end() ||
	       !(worldIt->second == catIt->second);
}
