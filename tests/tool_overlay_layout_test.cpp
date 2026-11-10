#include "gtest/gtest.h"

#include "world/CharacterLayout.h"
#include "world/ToolOverlayLayout.h"

namespace {

using world::CharacterLayout;
using world::FrameAnchor;
using world::SheetCell;
using world::Layer;
using world::LayerFrame;
using world::OverlayLayer;
using world::PixelRect;
using world::ToolOverlayAction;
using world::ToolOverlayCatalog;
using world::ToolOverlayFrame;
using world::ToolOverlayLayout;

LayerFrame makeLayer(int cols, int rows) {
	return LayerFrame{Layer::Body, SheetCell{0, 0}, cols, rows};
}

// Capa de un frame del overlay: la celda más su offset en celdas. Los
// tests que no distribuyen gotas usan offset (0,0).
OverlayLayer makeOverlayLayer(int cols, int rows) {
	return OverlayLayer{makeLayer(cols, rows), 0, 0};
}

ToolOverlayFrame makeFrame(FrameAnchor anchor) {
	ToolOverlayFrame frame;
	frame.anchor = anchor;
	frame.layers = {makeOverlayLayer(1, 2)};
	return frame;
}

ToolOverlayCatalog makeCatalog() {
	ToolOverlayCatalog catalog;
	catalog.anchorOffsets = {
		{FrameAnchor::Top, 14},
		{FrameAnchor::Body, 14},
		{FrameAnchor::Bottom, 14},
	};
	return catalog;
}

TEST(ToolOverlayLayoutTest, EmptyActionHasNoFrame) {
	const CharacterLayout body(PixelRect{0, 0, 128, 128}, 64, 64);
	const ToolOverlayCatalog catalog = makeCatalog();
	const ToolOverlayLayout layout(body, catalog, makeLayer(1, 2));

	ToolOverlayAction action;
	EXPECT_EQ(layout.frameAt(action, 0), nullptr);
}

TEST(ToolOverlayLayoutTest, FrameAdvancesWithPhaseAndClampsToLast) {
	const CharacterLayout body(PixelRect{0, 0, 128, 128}, 64, 64);
	const ToolOverlayCatalog catalog = makeCatalog();
	const ToolOverlayLayout layout(body, catalog, makeLayer(1, 2));

	ToolOverlayAction action;
	action.frameTicks = 10;
	action.frames = {makeFrame(FrameAnchor::Top),
			 makeFrame(FrameAnchor::Body),
			 makeFrame(FrameAnchor::Bottom)};

	EXPECT_EQ(layout.frameAt(action, 0), &action.frames[0]);
	EXPECT_EQ(layout.frameAt(action, 10), &action.frames[1]);
	EXPECT_EQ(layout.frameAt(action, 25), &action.frames[2]);
	EXPECT_EQ(layout.frameAt(action, 1000), &action.frames[2]);
}

TEST(ToolOverlayLayoutTest, NonPositiveFrameTicksUsesFirstFrame) {
	const CharacterLayout body(PixelRect{0, 0, 128, 128}, 64, 64);
	const ToolOverlayCatalog catalog = makeCatalog();
	const ToolOverlayLayout layout(body, catalog, makeLayer(1, 2));

	ToolOverlayAction action;
	action.frameTicks = 0;
	action.frames = {makeFrame(FrameAnchor::Top),
			 makeFrame(FrameAnchor::Body)};

	EXPECT_EQ(layout.frameAt(action, 99), &action.frames[0]);
}

TEST(ToolOverlayLayoutTest, AnchorSelectsTheBaseOfTheLayer) {
	const CharacterLayout body(PixelRect{0, 0, 128, 128}, 64, 64);
	const ToolOverlayCatalog catalog = makeCatalog();
	const ToolOverlayLayout layout(body, catalog, makeLayer(1, 2));

	const ToolOverlayFrame top = makeFrame(FrameAnchor::Top);
	const ToolOverlayFrame middle = makeFrame(FrameAnchor::Body);
	const ToolOverlayFrame bottom = makeFrame(FrameAnchor::Bottom);

	EXPECT_EQ(layout.layerRect(top, top.layers[0]).y,
		  body.bodyTop() - 64 + 14);
	EXPECT_EQ(layout.layerRect(middle, middle.layers[0]).y,
		  body.bodyTop() + 14);
	EXPECT_EQ(layout.layerRect(bottom, bottom.layers[0]).y,
		  body.bodyBottom() - 64 + 14);
}

TEST(ToolOverlayLayoutTest, OffsetsAndScaleAreApplied) {
	const CharacterLayout body(PixelRect{0, 0, 128, 128}, 64, 64);
	const ToolOverlayCatalog catalog = makeCatalog();
	const ToolOverlayLayout layout(body, catalog, makeLayer(1, 2));

	ToolOverlayFrame frame = makeFrame(FrameAnchor::Body);
	frame.layers = {makeOverlayLayer(1, 1)};
	frame.offsetX = 3;
	frame.offsetY = 5;
	frame.scale = 0.5;

	const PixelRect rect = layout.layerRect(frame, frame.layers[0]);
	EXPECT_EQ(rect.x, body.bodyX() + 3);
	EXPECT_EQ(rect.y, body.bodyTop() + 14 + 5);
	// El tamaño por celda sale de la capa base (1x2): una capa 1x1
	// mide UNA celda (64x32 acá) y la escala la achica proporcional.
	EXPECT_EQ(rect.width, 32);
	EXPECT_EQ(rect.height, 16);
}

TEST(ToolOverlayLayoutTest, InvalidLayerDoesNotDivideByZero) {
	const CharacterLayout body(PixelRect{0, 0, 128, 128}, 64, 64);
	const ToolOverlayCatalog catalog = makeCatalog();
	const ToolOverlayLayout layout(body, catalog, makeLayer(1, 2));

	ToolOverlayFrame frame = makeFrame(FrameAnchor::Top);
	frame.layers = {makeOverlayLayer(0, 1)};

	const PixelRect rect = layout.layerRect(frame, frame.layers[0]);
	EXPECT_LE(rect.width, 0);
	EXPECT_LE(rect.height, 0);
}

TEST(ToolOverlayLayoutTest, OneByOneLayerMeasuresOneBodyCell) {
	// El factor de píxeles por celda sale de la capa base (el cuerpo
	// 1x2), igual que CharacterLayout: una capa 1x1 (el huevo en las
	// manos) mide UNA celda del cuerpo, no el bloque entero.
	const CharacterLayout body(PixelRect{0, 0, 32, 64}, 32, 64);
	const ToolOverlayCatalog catalog = makeCatalog();
	const ToolOverlayLayout layout(body, catalog, makeLayer(1, 2));

	const ToolOverlayFrame frame = makeFrame(FrameAnchor::Body);
	const PixelRect rect = layout.layerRect(frame, makeOverlayLayer(1, 1));
	EXPECT_EQ(rect.width, 32);
	EXPECT_EQ(rect.height, 32);

	// Con escala se achica proporcional (el huevo va a media celda).
	ToolOverlayFrame scaled = makeFrame(FrameAnchor::Body);
	scaled.scale = 0.5;
	const PixelRect small = layout.layerRect(scaled, makeOverlayLayer(1, 1));
	EXPECT_EQ(small.width, 16);
	EXPECT_EQ(small.height, 16);
}

}  // namespace
