#include "world/PagedModal.h"

#include <algorithm>

#include "toml/TomlDocument.h"
#include "world/ConfigError.h"
#include "world/TomlConfigReader.h"

namespace world {

PagedModalSpec::PagedModalSpec() = default;

PagedModalSpec::PagedModalSpec(const tomlcfg::TomlDocument& doc) {
	const TomlConfigReader root(doc);

	const TomlConfigReader panel = root.subtable("panel");
	widthFraction = panel.fraction("width_fraction");
	heightFraction = panel.fraction("height_fraction");
	const PixelSize minSize = panel.positiveSize("min_size");
	minWidth = minSize.width;
	minHeight = minSize.height;
	if (panel.hasKey("max_size")) {
		const PixelSize maxSize = panel.positiveSize("max_size");
		maxWidth = maxSize.width;
		maxHeight = maxSize.height;
		if (maxWidth < minWidth || maxHeight < minHeight) {
			throw ConfigError(ConfigErrorKind::InvalidValue,
					  panel.pathFor("max_size"));
		}
	}
	framePad = panel.nonNegativeInt("frame_pad");
	bgInset = panel.nonNegativeInt("bg_inset");

	const TomlConfigReader arrows = root.subtable("arrows");
	const PixelSize arrowSize = arrows.positiveSize("size");
	arrowWidth = arrowSize.width;
	arrowHeight = arrowSize.height;
	arrowMargin = arrows.nonNegativeInt("margin");
	arrowGap = arrows.nonNegativeInt("gap");
	arrowLeftSheet = arrows.nonEmptyString("left_sheet");
	arrowRightSheet = arrows.nonEmptyString("right_sheet");
	arrowLeftFallback = arrows.nonEmptyString("left_fallback");
	arrowRightFallback = arrows.nonEmptyString("right_fallback");
	arrowFontSize = arrows.positiveInt("font_size");

	const TomlConfigReader colors = root.subtable("colors");
	backdropColor = colors.color("backdrop");
	parchmentColor = colors.color("parchment");
	woodBorderColor = colors.color("wood_border");
	inkColor = colors.color("ink");
}

PagedModalLayout::PagedModalLayout(const int screenW, const int screenH,
				   const PagedModalSpec& spec,
				   const int contentTopOffset,
				   const int contentBottomMargin)
	: bgInset_(spec.bgInset) {
	int panelW = std::max(
		spec.minWidth,
		static_cast<int>(static_cast<double>(screenW) *
				 spec.widthFraction));
	int panelH = std::max(
		spec.minHeight,
		static_cast<int>(static_cast<double>(screenH) *
				 spec.heightFraction));
	if (spec.maxWidth > 0) {
		panelW = std::min(panelW, spec.maxWidth);
	}
	if (spec.maxHeight > 0) {
		panelH = std::min(panelH, spec.maxHeight);
	}
	panel_ = PixelBox{
		(screenW - panelW) / 2,
		(screenH - panelH) / 2,
		panelW,
		panelH,
	};

	const int contentTop = panel_.y + contentTopOffset;
	const int contentBottom = panel_.y + panel_.height - contentBottomMargin;
	const int contentH = std::max(1, contentBottom - contentTop);
	const int arrowY = contentTop + (contentH - spec.arrowHeight) / 2;

	leftArrowRect_ = PixelBox{
		panel_.x + spec.arrowMargin,
		arrowY,
		spec.arrowWidth,
		spec.arrowHeight,
	};
	rightArrowRect_ = PixelBox{
		panel_.x + panel_.width - spec.arrowMargin - spec.arrowWidth,
		arrowY,
		spec.arrowWidth,
		spec.arrowHeight,
	};

	const int innerLeft =
		leftArrowRect_.x + leftArrowRect_.width + spec.arrowGap;
	const int innerRight = rightArrowRect_.x - spec.arrowGap;
	innerContentRect_ = PixelBox{
		innerLeft,
		contentTop,
		std::max(1, innerRight - innerLeft),
		contentH,
	};
}

const PixelBox& PagedModalLayout::panel() const { return panel_; }

PixelBox PagedModalLayout::backgroundBox() const {
	return panel_.inset(bgInset_);
}

const PixelBox& PagedModalLayout::leftArrowRect() const {
	return leftArrowRect_;
}

const PixelBox& PagedModalLayout::rightArrowRect() const {
	return rightArrowRect_;
}

const PixelBox& PagedModalLayout::innerContentRect() const {
	return innerContentRect_;
}

bool PagedModalLayout::hitsLeftArrow(const int x, const int y) const {
	return leftArrowRect_.contains(x, y);
}

bool PagedModalLayout::hitsRightArrow(const int x, const int y) const {
	return rightArrowRect_.contains(x, y);
}

bool PagedModalLayout::hitsAnyArrow(const int x, const int y) const {
	return hitsLeftArrow(x, y) || hitsRightArrow(x, y);
}

}  // namespace world
