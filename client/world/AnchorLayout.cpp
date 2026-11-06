#include "world/AnchorLayout.h"

namespace world {

AnchorLayout::AnchorLayout()
	: ops_{
		  {Anchor::TopLeft, {&AnchorLayout::leading, &AnchorLayout::leading}},
		  {Anchor::TopCenter, {&AnchorLayout::centered, &AnchorLayout::leading}},
		  {Anchor::TopRight, {&AnchorLayout::trailing, &AnchorLayout::leading}},
		  {Anchor::CenterLeft, {&AnchorLayout::leading, &AnchorLayout::centered}},
		  {Anchor::Center, {&AnchorLayout::centered, &AnchorLayout::centered}},
		  {Anchor::CenterRight, {&AnchorLayout::trailing, &AnchorLayout::centered}},
		  {Anchor::BottomLeft, {&AnchorLayout::leading, &AnchorLayout::trailing}},
		  {Anchor::BottomCenter, {&AnchorLayout::centered, &AnchorLayout::trailing}},
		  {Anchor::BottomRight, {&AnchorLayout::trailing, &AnchorLayout::trailing}},
	  }
	, names_{
		  {"top_left", Anchor::TopLeft},
		  {"top_center", Anchor::TopCenter},
		  {"top_right", Anchor::TopRight},
		  {"center_left", Anchor::CenterLeft},
		  {"center", Anchor::Center},
		  {"center_right", Anchor::CenterRight},
		  {"bottom_left", Anchor::BottomLeft},
		  {"bottom_center", Anchor::BottomCenter},
		  {"bottom_right", Anchor::BottomRight},
	  } {}

std::optional<Anchor> AnchorLayout::fromName(const std::string& name) const {
	const auto it = names_.find(name);
	if (it == names_.end()) {
		return std::nullopt;
	}
	return it->second;
}

int AnchorLayout::leading(int /*windowLen*/, int /*boxLen*/,
			  int margin) const {
	return margin;
}

int AnchorLayout::centered(int windowLen, int boxLen, int margin) const {
	return (windowLen - boxLen) / 2 + margin;
}

int AnchorLayout::trailing(int windowLen, int boxLen, int margin) const {
	return windowLen - boxLen - margin;
}

PixelBox AnchorLayout::box(Anchor anchor, int marginX, int marginY,
			   int width, int height, int windowWidth,
			   int windowHeight) const {
	const AxisOps& ops = ops_.at(anchor);
	PixelBox box;
	box.x = (this->*ops.x)(windowWidth, width, marginX);
	box.y = (this->*ops.y)(windowHeight, height, marginY);
	box.width = width;
	box.height = height;
	return box;
}

}
