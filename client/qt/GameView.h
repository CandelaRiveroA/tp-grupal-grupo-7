#pragma once

#include <QImage>
#include <QWidget>

namespace core {
class WindowConfig;
}

namespace qt {

class GameView : public QWidget {
	Q_OBJECT

public:
	explicit GameView(const core::WindowConfig& config, QWidget* parent = nullptr);

	QImage& frameBuffer();

	void present();

protected:
	void paintEvent(QPaintEvent* event) override;

private:
	QImage frame_;
};

}
