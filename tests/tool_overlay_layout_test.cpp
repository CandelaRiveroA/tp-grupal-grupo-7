#include "gtest/gtest.h"

#include "world/CharacterLayout.h"
#include "world/ToolOverlayLayout.h"

namespace {

using world::CharacterLayout;
using world::FrameAnchor;
using world::FrameIndex;
using world::Layer;
using world::LayerFrame;
using world::PixelRect;
using world::ToolOverlayAction;
using world::ToolOverlayCatalog;
using world::ToolOverlayFrame;
using world::ToolOverlayLayout;

LayerFrame makeLayer(int cols, int rows) {
	return LayerFrame{Layer::Body, FrameIndex{0, 0}, cols, rows};
}

ToolOverlayFrame makeFrame(FrameAnchor anchor) {
	ToolOverlayFrame frame;
	frame.anchor = anchor;
	frame.layers = {makeLayer(1, 2)};
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
	const ToolOverlayLayout layout(body, catalog);

	ToolOverlayAction action;
	EXPECT_EQ(layout.frameAt(action, 0), nullptr);
}

TEST(ToolOverlayLayoutTest, FrameAdvancesWithPhaseAndClampsToLast) {
	const CharacterLayout body(PixelRect{0, 0, 128, 128}, 64, 64);
	const ToolOverlayCatalog catalog = makeCatalog();
	const ToolOverlayLayout layout(body, catalog);

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
	const ToolOverlayLayout layout(body, catalog);

	ToolOverlayAction action;
	action.frameTicks = 0;
	action.frames = {makeFrame(FrameAnchor::Top),
			 makeFrame(FrameAnchor::Body)};

	EXPECT_EQ(layout.frameAt(action, 99), &action.frames[0]);
}

TEST(ToolOverlayLayoutTest, AnchorSelectsTheBaseOfTheLayer) {
	const CharacterLayout body(PixelRect{0, 0, 128, 128}, 64, 64);
	const ToolOverlayCatalog catalog = makeCatalog();
	const ToolOverlayLayout layout(body, catalog);

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
	const ToolOverlayLayout layout(body, catalog);

	ToolOverlayFrame frame = makeFrame(FrameAnchor::Body);
	frame.layers = {makeLayer(1, 1)};
	frame.offsetX = 3;
	frame.offsetY = 5;
	frame.scale = 0.5;

	const PixelRect rect = layout.layerRect(frame, frame.layers[0]);
	EXPECT_EQ(rect.x, body.bodyX() + 3);
	EXPECT_EQ(rect.y, body.bodyTop() + 14 + 5);
	EXPECT_EQ(rect.width, 32);
	EXPECT_EQ(rect.height, 32);
}

TEST(ToolOverlayLayoutTest, InvalidLayerDoesNotDivideByZero) {
	const CharacterLayout body(PixelRect{0, 0, 128, 128}, 64, 64);
	const ToolOverlayCatalog catalog = makeCatalog();
	const ToolOverlayLayout layout(body, catalog);

	ToolOverlayFrame frame = makeFrame(FrameAnchor::Top);
	frame.layers = {makeLayer(0, 1)};

	const PixelRect rect = layout.layerRect(frame, frame.layers[0]);
	EXPECT_LE(rect.width, 0);
	EXPECT_LE(rect.height, 0);
}

}  // namespace
