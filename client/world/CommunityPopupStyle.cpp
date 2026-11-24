#include "world/CommunityPopupStyle.h"

#include <algorithm>

namespace world {

int CommunityPopupStyle::setHeight(const CommunitySection& section) const {
	return headerHeight + static_cast<int>(section.rows.size()) * rowHeight;
}

int CommunityPopupStyle::contentHeight(
	const std::vector<CommunitySection>& sections) const {
	int height = 2 * inset + titleHeight;
	for (const CommunitySection& section : sections) {
		height += setGap + setHeight(section);
	}
	return height;
}

int CommunityPopupStyle::contentTop(
	const PixelBox& panel,
	const std::vector<CommunitySection>& sections) const {
	// Las filas del panel son de a un slot: si sobra alto, el contenido se
	// reparte parejo arriba y abajo.
	const int slack = std::max(0, panel.height - contentHeight(sections));
	return panel.y + slack / 2 + inset;
}

int CommunityPopupStyle::setTop(
	const PixelBox& panel, const std::vector<CommunitySection>& sections,
	const std::size_t section) const {
	int top = contentTop(panel, sections) + titleHeight;
	for (std::size_t i = 0; i < section; ++i) {
		top += setGap + setHeight(sections[i]);
	}
	return top + setGap;
}

PixelBox CommunityPopupStyle::titleBox(
	const PixelBox& panel,
	const std::vector<CommunitySection>& sections) const {
	return PixelBox{panel.x + inset, contentTop(panel, sections),
			std::max(1, panel.width - 2 * inset), titleHeight};
}

PixelBox CommunityPopupStyle::headerBox(
	const PixelBox& panel, const std::vector<CommunitySection>& sections,
	const std::size_t section) const {
	return PixelBox{panel.x + inset, setTop(panel, sections, section),
			std::max(1, panel.width - 2 * inset), headerHeight};
}

PixelBox CommunityPopupStyle::rowBox(
	const PixelBox& panel, const std::vector<CommunitySection>& sections,
	const std::size_t section, const std::size_t row) const {
	return PixelBox{panel.x + inset,
			setTop(panel, sections, section) + headerHeight +
				static_cast<int>(row) * rowHeight,
			std::max(1, panel.width - 2 * inset), rowHeight};
}

PixelBox CommunityPopupStyle::statusBox(const PixelBox& header,
					const int titleWidth) const {
	const int left = titleWidth + statusGap;
	return PixelBox{header.x + left, header.y,
			std::max(1, header.width - left), header.height};
}

PixelBox CommunityPopupStyle::badgeBox(const PixelBox& header,
				       const int titleWidth) const {
	const int width = setDone.displayWidth();
	const int height = setDone.displayHeight();
	return PixelBox{header.x + titleWidth + statusGap,
			header.y + (header.height - height) / 2, width, height};
}

PixelBox CommunityPopupStyle::checkboxBox(const PixelBox& row) const {
	const int width = checkboxEmpty.displayWidth();
	const int height = checkboxEmpty.displayHeight();
	return PixelBox{row.x, row.y + (row.height - height) / 2, width,
			height};
}

PixelBox CommunityPopupStyle::labelBox(const PixelBox& row) const {
	const int left = checkboxEmpty.displayWidth() + checkboxGap;
	return PixelBox{row.x + left, row.y,
			std::max(1, row.width - left - amountWidth), row.height};
}

PixelBox CommunityPopupStyle::amountBox(const PixelBox& row,
					const int textWidth) const {
	const int width = std::min(textWidth, amountWidth);
	return PixelBox{row.x + row.width - width, row.y, std::max(1, width),
			row.height};
}

}
