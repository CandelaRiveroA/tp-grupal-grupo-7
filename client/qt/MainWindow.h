#pragma once

#include <QWidget>

class QKeyEvent;
class QFocusEvent;

namespace core {
class WindowConfig;
class InputManager;
}

namespace qt {

class MainWindow : public QWidget {
	Q_OBJECT

public:
	explicit MainWindow(const core::WindowConfig& config,
			    core::InputManager& input,
			    QWidget* parent = nullptr);

	void setContent(QWidget* widget);

protected:
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;
	void focusOutEvent(QFocusEvent* event) override;

private:
	core::InputManager& input_;
};

}
