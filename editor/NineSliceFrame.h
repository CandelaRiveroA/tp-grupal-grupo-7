#pragma once

#include <QMargins>
#include <QPixmap>
#include <QString>
#include <QWidget>

#include "qt_theme/NineSlice.h"

class QPaintEvent;
class QResizeEvent;

// Marco de madera 9-slice escalado (pixel art nitido) que hace de panel:
// el que lo crea le pone un layout y sus hijos quedan dentro del borde.
class NineSliceFrame : public QWidget {
public:
	NineSliceFrame(const QString& resource, const QMargins& slice, int scale,
		       QWidget* parent = nullptr);

protected:
	void paintEvent(QPaintEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;

private:
	NineSlice frame_;
	int scale_;
	QPixmap cached_;

	void rebuild();
};
