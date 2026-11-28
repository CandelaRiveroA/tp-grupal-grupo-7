#include "WorkspaceView.h"

#include <exception>
#include <vector>

#include <QAbstractButton>
#include <QButtonGroup>
#include <QDialog>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QSize>
#include <QSplitter>
#include <QStringList>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <algorithm>
#include <utility>

#include "AddExtraMapDialog.h"
#include "Constants.h"
#include "EditorDialog.h"
#include "MapCanvasView.h"
#include "MapTreeWidget.h"
#include "MinimapGenerator.h"
#include "PaletteView.h"
#include "PortalsView.h"
#include "ScenarioLabel.h"
#include "ValidationPanel.h"
#include "qt_theme/GameCursor.h"
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
	setObjectName("workspaceView");
	setAttribute(Qt::WA_StyledBackground);
	try {
		catalog_ = worldmodel::PrefabCatalogLoader().loadFromFile(
			editor::kPrefabsTomlPath);
	} catch (const std::exception& error) {
		EditorDialog(this).info("No se pudo cargar prefabs",
					QString::fromStdString(error.what()));
	}
	tilesets_.loadTilesets(catalog_.tilesets());

	tree_ = new MapTreeWidget(this);
	canvas_ = new MapCanvasView(catalog_, tilesets_, markerSprites_, this);
	palette_ = new PaletteView(catalog_, tilesets_, markerSprites_, this);
	portals_ = new PortalsView(catalog_, tilesets_, this);
	rightTabs_ = new QTabWidget(this);
	rightTabs_->setObjectName("rightTabs");
	rightTabs_->addTab(palette_, "Prefabs");
	rightTabs_->addTab(portals_, "Portales");

	// La validacion vive en un dialogio
	validationDialog_ = new QDialog(this);
	validationDialog_->setObjectName("validationDialog");
	validationDialog_->setCursor(GameCursor().cursor());
	validationDialog_->setWindowTitle("Validacion del mundo");
	validationPanel_ = new ValidationPanel(validationDialog_);
	QVBoxLayout* dialogLayout = new QVBoxLayout(validationDialog_);
	dialogLayout->addWidget(validationPanel_);
	validationDialog_->resize(editor::kValidationDialogSize);

	QSplitter* bodySplit = new QSplitter(Qt::Horizontal, this);
	bodySplit->addWidget(tree_);
	bodySplit->addWidget(canvas_);
	bodySplit->addWidget(rightTabs_);
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
	connect(palette_, &PaletteView::markerSelected, this,
		&WorkspaceView::onMarkerSelected);
	connect(palette_, &PaletteView::portalPicked, this,
		&WorkspaceView::onPortalPicked);
	connect(canvas_, &MapCanvasView::placementRejected, this,
		[this](const QString& title, const QString& message) {
			EditorDialog(this).info(title, message);
		});
	connect(portals_, &PortalsView::portalPicked, this,
		&WorkspaceView::onPortalPicked);
	connect(canvas_, &MapCanvasView::cabinErased, this,
		[this](int cabinIndex) { pendingErasedCabin_ = cabinIndex; });
	connect(canvas_, &MapCanvasView::mapEdited, this, [this] {
		if (world_.maps.contains(activeMapId_)) {
			for (const auto& obj :
			     world_.maps.at(activeMapId_).objects) {
				ensureWorldTileset(obj.tile.tileset);
			}
		}
		if (activeMapId_ == editor::kHomeMapId) {
			syncFarmCabins(pendingErasedCabin_);
		}
		if (rules_.isInteriorHost(activeMapId_)) {
			worldmodel::WorldFactory().wirePortals(world_);
		}
		pendingErasedCabin_.reset();
		setDirty();
	});
	connect(canvas_, &MapCanvasView::portalTargetRequested, this,
		&WorkspaceView::onPortalTargetRequested);
	connect(canvas_, &MapCanvasView::targetCellPicked, this,
		&WorkspaceView::onTargetCellPicked);
	connect(canvas_, &MapCanvasView::pickTargetCancelled, this,
		&WorkspaceView::onPickTargetCancelled);
	connect(canvas_, &MapCanvasView::toolResetToNavigate, this,
		[this] { navigateButton_->setChecked(true); });
}

QToolButton* WorkspaceView::makeToolButton(const QString& text,
					   const QString& icon,
					   bool checkable) {
	auto* btn = new QToolButton(this);
	btn->setText(text);
	btn->setToolTip(text);
	btn->setIcon(QIcon(icon));
	btn->setIconSize(QSize(editor::kToolIconSize, editor::kToolIconSize));
	btn->setCheckable(checkable);
	btn->setObjectName("editorToolButton");
	return btn;
}

QHBoxLayout* WorkspaceView::buildTopBar() {
	heading_ = new QLabel(this);
	heading_->setObjectName("worldHeading");
	heading_->setIndent(editor::kHeadingIndent);
	const QSize iconSize(editor::kToolIconSize, editor::kToolIconSize);
	QPushButton* backButton = new QPushButton("Volver al inicio", this);
	backButton->setIcon(QIcon(":/editor/ui/icon_back.png"));
	backButton->setIconSize(iconSize);
	backButton->setObjectName("editorButton");
	QPushButton* validateButton = new QPushButton("Validar", this);
	validateButton->setIcon(QIcon(":/editor/ui/icon_validate.png"));
	validateButton->setIconSize(iconSize);
	validateButton->setObjectName("editorButton");
	QPushButton* saveButton = new QPushButton("Guardar", this);
	saveButton->setIcon(QIcon(":/editor/ui/icon_save.png"));
	saveButton->setIconSize(iconSize);
	saveButton->setObjectName("editorButton");
	QToolButton* mapsButton = new QToolButton(this);
	mapsButton->setText("Mapas");
	mapsButton->setCheckable(true);
	mapsButton->setChecked(true);
	mapsButton->setObjectName("editorToolButton");

	navigateButton_ = makeToolButton("Navegar",
					 ":/editor/ui/tool_navigate.png", true);
	navigateButton_->setChecked(true);
	placeButton_ =
		makeToolButton("Colocar", ":/editor/ui/tool_place.png", true);
	eraseButton_ =
		makeToolButton("Borrar", ":/editor/ui/tool_erase.png", true);
	moveButton_ =
		makeToolButton("Mover", ":/editor/ui/tool_move.png", true);

	toolGroup_ = new QButtonGroup(this);
	toolGroup_->setExclusive(true);
	toolGroup_->addButton(navigateButton_,
			      static_cast<int>(EditorTool::Navigate));
	toolGroup_->addButton(placeButton_,
			      static_cast<int>(EditorTool::Place));
	toolGroup_->addButton(eraseButton_,
			      static_cast<int>(EditorTool::Erase));
	toolGroup_->addButton(moveButton_,
			      static_cast<int>(EditorTool::Move));

	QToolButton* collisionButton = makeToolButton(
		"Colision", ":/editor/ui/icon_collision.png", true);
	QToolButton* zoomOutButton = makeToolButton(
		"Alejar", ":/editor/ui/icon_zoom_out.png", false);
	QToolButton* zoomResetButton = makeToolButton(
		"100%", ":/editor/ui/icon_zoom_reset.png", false);
	QToolButton* zoomInButton = makeToolButton(
		"Acercar", ":/editor/ui/icon_zoom_in.png", false);

	QHBoxLayout* top = new QHBoxLayout;
	top->setContentsMargins(editor::kTopBarMargins);
	top->addWidget(mapsButton);
	top->addWidget(heading_);
	top->addStretch();
	top->addWidget(navigateButton_);
	top->addWidget(placeButton_);
	top->addWidget(eraseButton_);
	top->addWidget(moveButton_);
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
	connect(toolGroup_, &QButtonGroup::idToggled, this,
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
		if (dirty_ && !EditorDialog(this).confirm(
				      "Cambios sin guardar",
				      "Hay cambios sin guardar. Los "
				      "descarto y vuelvo al inicio?")) {
			return;
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
	pendingErasedCabin_.reset();
	pendingPortal_.reset();
	setPickTargetUiLocked(false);
	world_ = world;
	dirty_ = false;
	palette_->clearSelection();
	portals_->clearSelection();
	canvas_->clearPlacement();
	navigateButton_->setChecked(true);
	canvas_->setTool(EditorTool::Navigate);
	tilesets_.loadTilesets(world_.tilesets);
	tilesets_.loadTilesets(catalog_.tilesets());
	validationPanel_->showIssues({});
	updateHeading();
	tree_->setExtraMaps(world_);
	selectMap(editor::kHomeMapId);
}

void WorkspaceView::selectMap(const std::string& mapId) {
	activeMapId_ = mapId;
	tree_->selectMap(mapId);
	worldmodel::Map* mapPtr =
		world_.maps.contains(mapId) ? &world_.maps.at(mapId) : nullptr;
	canvas_->setMap(mapPtr);
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
		EditorDialog(this).info("No se pudo guardar",
					QString::fromStdString(error.what()));
		return;
	}
	validationPanel_->showIssues(issues);
	if (!issues.empty()) {
		EditorDialog(this).info(
			"Guardar bloqueado",
			"No se puede guardar: el validador reporto " +
				QString::number(issues.size()) +
				" problema(s). Corregilos antes de guardar.");
		return;
	}
	dirty_ = false;
	updateHeading();
	const QString savedPath =
		QString::fromStdString(worlds_.filePathFor(world_.name));
	QStringList failedMaps;
	if (MinimapGenerator().generate(world_, tilesets_, worlds_,
					failedMaps)) {
		EditorDialog(this).info("Mundo guardado",
					"Guardado en " + savedPath);
	} else {
		const QString detail =
			failedMaps.isEmpty()
				? "no se pudo preparar la carpeta de minimapas."
				: "fallaron los minimapas: " +
					  failedMaps.join(", ") + ".";
		EditorDialog(this).info("Minimapa no generado",
					"El mundo se guardo en " + savedPath +
						", pero " + detail);
	}
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
		EditorDialog(this).info(
			"No se pudo agregar el mapa",
			QString::fromStdString(error.what()));
		return;
	}
	tilesets_.loadTilesets(world_.tilesets);
	setDirty();
	tree_->setExtraMaps(world_);
	selectMap(dialog.mapId());
}

void WorkspaceView::removeExtraMap() {
	if (!EditorDialog(this).confirm(
		    "Eliminar mapa extra",
		    QString::fromStdString("Elimino el mapa " + activeMapId_ +
					   "? No hay deshacer."))) {
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
		EditorDialog(this).info("Cambiar escenario base",
					"No hay escenarios disponibles.");
		return;
	}

	QStringList labels;
	for (const worldmodel::Scenario* scenario : options) {
		labels << scenarioLabel(*scenario);
	}
	const std::optional<QString> chosen = EditorDialog(this).pickItem(
		"Cambiar escenario base", "Nuevo escenario:", labels);
	if (!chosen.has_value()) {
		return;
	}
	const worldmodel::Scenario& scenario =
		*options[labels.indexOf(*chosen)];

	if (!EditorDialog(this).confirm(
		    "Cambiar escenario base",
		    QString::fromStdString(
			    "Reemplazar el escenario base de " + activeMapId_ +
			    " por " + scenario.id +
			    "? Se reinicia el terreno y la colision del mapa" +
			    (mandatory
				     ? " y se recolocan sus puntos por defecto"
				     : "") +
			    "."))) {
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
		EditorDialog(this).info(
			"No se pudo reemplazar",
			QString::fromStdString(error.what()));
		return;
	}
	tilesets_.loadTilesets(world_.tilesets);
	tilesets_.loadTilesets(catalog_.tilesets());
	setDirty();
	worldmodel::Map* mapPtr = &world_.maps.at(activeMapId_);
	canvas_->setMap(mapPtr);
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
		canvas_->clearPlacement();
		return;
	}
	canvas_->setActivePrefab(prefab);
	placeButton_->setChecked(true);
	canvas_->setTool(EditorTool::Place);
}

void WorkspaceView::onMarkerSelected(worldmodel::MarkerKind kind) {
	canvas_->setPlacingMarker(kind);
	placeButton_->setChecked(true);
	canvas_->setTool(EditorTool::Place);
}

void WorkspaceView::onPortalPicked(worldmodel::PortalKind kind, int width,
				   int height) {
	uncheckToolButtons();
	canvas_->setActivePortal(kind, width, height);
}

void WorkspaceView::onPortalTargetRequested(std::size_t portalIndex) {
	if (!world_.maps.contains(activeMapId_)) {
		return;
	}
	const auto& portals = world_.maps.at(activeMapId_).portals;
	if (portalIndex >= portals.size()) {
		return;
	}
	const auto& portal = portals[portalIndex];
	std::optional<std::string> targetMap;
	if (portal.kind == worldmodel::PortalKind::Link) {
		QStringList mapIds;
		for (const auto& [id, map] : world_.maps) {
			mapIds << QString::fromStdString(id);
		}
		const std::optional<QString> chosen = EditorDialog(this).pickItem(
			"Portal link", "Mapa destino:", mapIds);
		if (!chosen.has_value() || chosen->isEmpty()) {
			return;
		}
		targetMap = chosen->toStdString();
	} else {
		targetMap =
			rules_.expectedPortalTarget(activeMapId_, portal.kind);
	}
	if (!targetMap.has_value() || !world_.maps.contains(*targetMap)) {
		return;
	}

	pendingPortal_ = PendingPortalTarget{activeMapId_, portalIndex};
	setPickTargetUiLocked(true);
	selectMap(*targetMap);
	canvas_->setPickTargetSize(portal.width, portal.height);
}

void WorkspaceView::onTargetCellPicked(worldmodel::TilePos cell) {
	if (!pendingPortal_.has_value()) {
		return;
	}
	const PendingPortalTarget pending = *pendingPortal_;
	pendingPortal_.reset();
	if (world_.maps.contains(pending.originMapId)) {
		auto& portals = world_.maps.at(pending.originMapId).portals;
		if (pending.portalIndex < portals.size()) {
			portals[pending.portalIndex].targetMap = activeMapId_;
			portals[pending.portalIndex].targetCell = cell;
			setDirty();
		}
	}
	setPickTargetUiLocked(false);
	navigateButton_->setChecked(true);
	canvas_->setTool(EditorTool::Navigate);
	selectMap(pending.originMapId);
	updateHeading();
}

void WorkspaceView::onPickTargetCancelled() {
	if (!pendingPortal_.has_value()) {
		return;
	}
	const std::string originMapId = pendingPortal_->originMapId;
	pendingPortal_.reset();
	setPickTargetUiLocked(false);
	navigateButton_->setChecked(true);
	canvas_->setTool(EditorTool::Navigate);
	selectMap(originMapId);
	updateHeading();
}

void WorkspaceView::syncFarmCabins(std::optional<int> erasedCabinIndex) {
	if (erasedCabinIndex.has_value() &&
	    *erasedCabinIndex >= worldmodel::kMinPlayers &&
	    *erasedCabinIndex <= worldmodel::kMaxPlayers) {
		world_.maps.erase(rules_.cabinId(*erasedCabinIndex));
		for (int k = *erasedCabinIndex + 1;
		     k <= worldmodel::kMaxPlayers; ++k) {
			const std::string oldId = rules_.cabinId(k);
			const std::string newId = rules_.cabinId(k - 1);
			if (world_.maps.contains(oldId)) {
				auto node = world_.maps.extract(oldId);
				node.key() = newId;
				node.mapped().id = newId;
				world_.maps.insert(std::move(node));
			}
		}
	}

	const auto& objects = world_.maps.at(editor::kHomeMapId).objects;
	const int cabinCount = static_cast<int>(std::count_if(
		objects.begin(), objects.end(), [](const auto& obj) {
			return obj.role == worldmodel::ObjectRole::Cabin;
		}));
	const auto cabinScenarios =
		scenarios_.byKind(worldmodel::MapKind::Cabin);
	for (int k = worldmodel::kMinPlayers;
	     k <= std::min(cabinCount, worldmodel::kMaxPlayers); ++k) {
		const std::string id = rules_.cabinId(k);
		if (!world_.maps.contains(id) && !cabinScenarios.empty()) {
			worldmodel::WorldFactory().addCabinMap(
				world_, k, *cabinScenarios.front());
			tilesets_.loadTilesets(world_.tilesets);
		}
	}
	tree_->setExtraMaps(world_);
}

void WorkspaceView::uncheckToolButtons() {
	toolGroup_->setExclusive(false);
	for (QAbstractButton* button : toolGroup_->buttons()) {
		button->setChecked(false);
	}
	toolGroup_->setExclusive(true);
}

void WorkspaceView::setPickTargetUiLocked(bool locked) {
	tree_->setEnabled(!locked);
	rightTabs_->setEnabled(!locked);
	navigateButton_->setEnabled(!locked);
	placeButton_->setEnabled(!locked);
	eraseButton_->setEnabled(!locked);
	moveButton_->setEnabled(!locked);
	if (locked) {
		uncheckToolButtons();
		heading_->setText("Elegi la celda destino (Esc cancela)");
	}
}

void WorkspaceView::setDirty() {
	dirty_ = true;
	updateHeading();
}

void WorkspaceView::updateHeading() {
	heading_->setText(QString::fromStdString(world_.name +
						 (dirty_ ? " *" : "")));
}

void WorkspaceView::ensureWorldTileset(const std::string& tilesetId) {
	const auto catIt = catalog_.tilesets().find(tilesetId);
	if (catIt != catalog_.tilesets().end()) {
		world_.tilesets.insert_or_assign(tilesetId, catIt->second);
	}
}
