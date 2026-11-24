#include "NineSliceFrame.h"

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QtGlobal>

NineSliceFrame::NineSliceFrame(const QString& resource, const QMargins& slice,
			       int scale, QWidget* parent)
	: QWidget(parent), frame_(QPixmap(resource), slice), scale_(scale) {
	if (QPixmap(resource).isNull()) {
		qWarning("NineSliceFrame: %s no carga", qPrintable(resource));
	}
	setContentsMargins(slice * scale);
	setMinimumSize((slice.left() + slice.right()) * scale,
		       (slice.top() + slice.bottom()) * scale);
}

void NineSliceFrame::paintEvent(QPaintEvent*) {
	if (cached_.isNull()) {
		rebuild();
	}
	QPainter(this).drawPixmap(0, 0, cached_);
}

void NineSliceFrame::resizeEvent(QResizeEvent*) { rebuild(); }

void NineSliceFrame::rebuild() {
	const int originalWidth = width() / scale_;
	const int originalHeight = height() / scale_;
	if (originalWidth <= 0 || originalHeight <= 0) {
		cached_ = QPixmap();
		return;
	}
	cached_ = frame_.build(originalWidth, originalHeight, scale_);
}
