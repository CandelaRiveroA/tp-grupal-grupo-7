#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "world/ViewDirection.h"
#include "world/CharacterAppearance.h"
#include "world/ItemKind.h"

namespace world {

// Anclaje vertical del frame del overlay respecto al cuerpo: arriba de la
// cabeza, a nivel del cuerpo o a los pies.
enum class FrameAnchor {
	Top,
	Body,
	Bottom,
};

// Geometría de un frame del overlay: dónde se apoya el sprite respecto
// del cuerpo y cómo se ajusta (offsets, escala, rotación, espejado). La
// comparten lo que dibuja una hoja fija (las herramientas) y lo que
// dibuja el contenido sostenido (la cosecha al recoger).
struct OverlayFrame {
	FrameAnchor anchor = FrameAnchor::Top;
	int offsetX = 0;
	int offsetY = 0;
	double angle = 0.0;
	double pivotX = 0.5;
	double pivotY = 0.5;
	bool flip = false;
	double scale = 1.0;
	// Caja del patrón EN CELDAS (ancho, alto). Si es > 0, el frame se
	// dibuja NORMALIZADO para que todo su patrón de capas entre en UNA
	// celda lógica: cada capa mide `celda / cellSpan` y los offsets en
	// celdas usan ese sub-tamaño. Así un grupo de 1x2 y uno de 4x4
	// ocupan el MISMO tamaño en pantalla (el chorro no crece al crecer
	// el patrón). 0 = sin normalizar: cada capa mide una celda entera
	// (el caso de la herramienta de una sola celda).
	int cellSpanX = 0;
	int cellSpanY = 0;
};

// Una capa de un frame del overlay: la celda (o bloque) de la hoja más
// su desplazamiento EN CELDAS respecto del ancla del frame. El offset
// permite que un frame con VARIAS capas las distribuya en 2D (el chorro
// de la regadera: cada gota cae en su posición), en vez de apilarlas en
// el mismo punto. Una capa sin offset (0,0) se comporta como antes: es
// el caso de la herramienta de una sola celda (pico, pala).
struct OverlayLayer {
	LayerFrame layer;
	int gridOffsetX = 0;
	int gridOffsetY = 0;
};

// Un frame del overlay de una herramienta: su geometría + las capas de
// la hoja que se dibujan en él (cada una es una celda del sheet con su
// offset en celdas; vacío = sin herramienta en ese cuadro).
struct ToolOverlayFrame : OverlayFrame {
	std::vector<OverlayLayer> layers;
};

// Un frame del overlay de lo que se lleva en las manos como CONTENIDO
// (la cosecha al recoger): la misma geometría, pero el sprite dibujado
// es el del contenido que se levanta. `drawn` permite ocultarlo en un
// cuadro (el mismo truco que los frames sin capas del huevo de
// espaldas).
struct HeldOverlayFrame : OverlayFrame {
	bool drawn = true;
};

// Un overlay animado de una hoja fija: su hoja, celda, ritmo, la acción
// con la que se sincroniza y sus frames (cada uno con sus capas). Es el
// dato común de TODO lo que se anima encima del personaje leyendo una
// hoja: la herramienta en uso y —el mismo mecanismo— el efecto que la
// herramienta larga (el chorro de la regadera). No se reimplementa la
// animación por caso: se reutiliza este tipo.
struct AnimatedOverlay {
	std::string sheetImage;
	int cellWidth = 0;
	int cellHeight = 0;
	int frameTicks = 0;
	// Acción con la que se sincroniza este overlay (dato): el mismo
	// mecanismo dibuja la herramienta en use_tool y lo que se lleva en
	// las manos en otras acciones (el huevo al recoger). El número de
	// frames y los frame_ticks deben coincidir con los de ESA acción en
	// characters.toml.
	ActionKind action = ActionKind::UseTool;
	std::vector<ToolOverlayFrame> frames;
};

// El overlay de una herramienta: su animación (AnimatedOverlay) y, de
// forma OPCIONAL, un EFECTO que larga mientras hace su acción (otro
// overlay animado, con su propia hoja): el chorro de la regadera al
// regar. El efecto es genérico por datos: mañana otra herramienta
// declara su `effect` sin tocar el dibujo. Que sea un AnimatedOverlay
// (no un ToolOverlayAction) corta la recursión del tipo y mantiene
// semántica de valor (el catálogo se copia/mueve).
struct ToolOverlayAction : AnimatedOverlay {
	std::optional<AnimatedOverlay> effect;
};

// Overlay de lo que se lleva en las manos cuando es un CONTENIDO
// genérico (la cosecha): la misma animación de recogida que el huevo,
// pero el sprite es el del contenido que se levanta (el mismo del
// inventario), no una hoja fija. Por eso es común a todos los
// contenidos: una cosecha nueva no agrega datos acá.
struct HeldContentOverlay {
	int frameTicks = 0;
	ActionKind action = ActionKind::Pickup;
	std::vector<HeldOverlayFrame> frames;
};

using ToolOverlayAppearance = std::map<Direction, ToolOverlayAction>;

struct ToolOverlayCatalog {
	// Ajuste vertical (px) por anclaje: cada uno se afina por separado
	// porque el gap transparente de la hoja difiere según dónde se apoya.
	std::map<FrameAnchor, int> anchorOffsets;

	std::map<ItemKind, ToolOverlayAppearance> tools;

	// Overlay de lo que se lleva en las manos como contenido (la
	// cosecha), por dirección: misma geometría que una herramienta,
	// dibujando el sprite del contenido sostenido.
	std::map<Direction, HeldContentOverlay> heldContent;

	// Una sola búsqueda: nullptr si (kind, dir) no tiene overlay.
	const ToolOverlayAction* find(ItemKind kind, Direction dir) const;

	// Overlay del contenido sostenido en las manos para esa dirección
	// (nullptr si no está declarado).
	const HeldContentOverlay* findHeldContent(Direction dir) const;

	int offsetFor(FrameAnchor anchor) const;
};

}
