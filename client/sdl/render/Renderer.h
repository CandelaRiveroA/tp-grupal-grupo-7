#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "game/ActionResolver.h"
#include "game/LocalWorldView.h"
#include "game/ViewMotion.h"
#include "game/ViewSpatialIndex.h"
#include "sdl/render/EntityRenderer.h"
#include "sdl/render/CropPainter.h"
#include "game/NpcPlacementResolver.h"
#include "sdl/render/GroundEffectPainter.h"
#include "sdl/render/NpcRenderer.h"
#include "sdl/render/PropPainter.h"
#include "game/FurnitureKey.h"
#include "sdl/render/SheetCache.h"
#include "sdl/render/TileLayerPainter.h"
#include "world/ContentCatalog.h"
#include "world/FurnitureGeometry.h"
#include "world/DepthOrder.h"
#include "world/CropCatalog.h"
#include "world/GroundCatalog.h"
#include "world/ResourceCatalog.h"
#include "world/SceneDescription.h"
#include "world/SceneElementIndex.h"
#include "world/ScenePixelMapper.h"

namespace world {
struct CharacterPalettes;
}

namespace sdl {
class SdlContext;
class Viewport;
}

namespace sdl {

// Orquestador del frame de mundo: limpia, consulta qué está visible y delega
// cada capa de dibujo en su pintor. No conoce cómo se dibuja una entidad.
class Renderer {
public:
	Renderer(SdlContext& context, const world::CharacterCatalog& characters,
		 const world::ToolOverlayCatalog& toolOverlay,
		 const world::ResourceCatalog& resources,
		 const world::ContentCatalog& content,
		 const world::GroundCatalog& ground,
		 const world::CropCatalog& crops,
		 const world::CharacterPalettes& palettes);

	// Las hojas cacheadas del mundo (una textura por imagen): quien
	// dibuja sobre el mundo con los mismos sprites las comparte.
	SheetCache& sheets();

	// Reconstruye el índice de elementos estáticos. Se llama cuando se
	// entra a una escena o cuando cambia su contenido, no por frame.
	void prepareScene(const world::SceneDescription& scene);

	// NPCs de la sala actual: se dibujan antes de las entidades.
	void setNpcs(const std::vector<const world::NpcDef*>& npcs);
	// Cuadro actual de los NPCs animados (cofres), por id.
	// También lo usan los muebles animados colocados (clave
	// game::FurnitureKey).
	void setNpcFrames(const std::map<std::string, int>& frames);

	// Geometría de los muebles (bloque o por piezas, con su ancla):
	// la MISMA que usan el click y la colocación. Debe vivir más que el
	// Renderer; sin ella los muebles no se dibujan.
	void setFurniture(const world::FurnitureGeometry& furniture);

	// Las filas frontales de un mueble por piezas (la cama: lo que tapa a
	// quien duerme), encima de las entidades. QUÉ filas lo decide
	// game::EnterSelection.
	void drawFurnitureFront(const game::ViewFurniture& furniture,
				const world::ScenePixelMapper& mapper,
				const Viewport& viewport, int firstRow,
				int lastRow);

	// `timeOfDay` filtra los elementos con `when` (scenes.toml):
	// qué momento es lo dice el dueño de DayPhase, no esto.
	void drawWorld(const world::SceneDescription& scene,
		       const game::LocalWorldView& view,
		       const game::ViewMotion& motion,
		       const game::ActionResolver& actionResolver,
		       int animationTick, const Viewport& viewport,
		       world::TimeOfDay timeOfDay);

	// Capa frontal de un elemento: solo el rango de filas de su pieza,
	// ya sobre las entidades (lo que "tapa" al jugador metido adentro).
	// QUÉ y CUÁNDO dibujar lo decide la capa de presentación; esto
	// solo pinta, con el mismo culling por bounds que todo elemento.
	void drawElementFront(const world::SceneDescription& scene,
			      const world::ElementPlacement& element,
			      const world::ScenePixelMapper& mapper,
			      const Viewport& viewport, int firstRow,
			      int lastRow);

	const PlayerAppearancePainter& appearancePainter() const;

private:
	void drawResource(const game::ViewResource& resource,
			  const world::ScenePixelMapper& mapper,
			  const Viewport& viewport);

	// Un mueble colocado (por contentId, con el visual de inventory.toml)
	// en su caja de mundo.
	void drawFurniture(const game::ViewFurniture& furniture,
			   const world::ScenePixelMapper& mapper,
			   const Viewport& viewport);

	void drawDroppedItem(const game::ViewDroppedItem& item,
			     const world::ScenePixelMapper& mapper,
			     const Viewport& viewport);

	void drawElement(const world::SceneDescription& scene,
			 const world::ElementPlacement& element,
			 const world::ScenePixelMapper& mapper,
			 const Viewport& viewport);

	// Setup común de un dibujo de elemento: hoja cacheada, painter en
	// coordenadas de pantalla y solo las filas del rango pedido.
	void drawElementRange(const world::SceneDescription& scene,
			      const world::ElementPlacement& element,
			      const world::ScenePixelMapper& mapper,
			      const Viewport& viewport, int firstRow,
			      int lastRow);

	SdlContext& context_;

	const world::ResourceCatalog& resources_;
	const world::ContentCatalog& content_;
	const world::GroundCatalog& ground_;

	SheetCache sheetCache_;
	EntityRenderer entityRenderer_;
	TileLayerPainter layerPainter_;
	NpcRenderer npcRenderer_;
	PropPainter propPainter_;
	game::FurnitureKey furnitureKey_;
	const world::FurnitureGeometry* furniture_ = nullptr;
	std::map<std::string, int> propFrames_;
	game::NpcPlacementResolver npcPlacements_;
	GroundEffectPainter groundPainter_;
	CropPainter cropPainter_;

	world::SceneElementIndex sceneIndex_;
	// Orden de dibujo en profundidad del frame (elementos de pie y
	// entidades); se reutiliza para no reservar memoria por frame.
	world::DepthOrder depth_;
	game::ViewSpatialIndex viewIndex_;
	bool sceneIndexReady_ = false;

	std::vector<std::uint32_t> visibleElements_;
	std::vector<std::uint32_t> visibleResources_;
	std::vector<std::uint32_t> visibleItems_;
	std::vector<std::uint32_t> visibleEntities_;
};

}  // namespace sdl
