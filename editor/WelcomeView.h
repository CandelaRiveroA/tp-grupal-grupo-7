#pragma once

#include <string>

#include <QBasicTimer>
#include <QLabel>
#include <QListWidget>
#include <QWidget>

#include "qt_theme/TitleBackdrop.h"
#include "world_model/WorldRepository.h"

class QHideEvent;
class QPaintEvent;
class QResizeEvent;
class QShowEvent;
class QTimerEvent;
class TitleButton;

// Pantalla inicial
class WelcomeView : public QWidget {
	Q_OBJECT
public:
	explicit WelcomeView(const worldmodel::WorldRepository& repository,
			     QWidget* parent = nullptr);

	void refresh();

signals:
	void newWorldRequested();
	void openWorldRequested(const std::string& name);

protected:
	void paintEvent(QPaintEvent* event) override;
	void timerEvent(QTimerEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;
	void showEvent(QShowEvent* event) override;
	void hideEvent(QHideEvent* event) override;

private:
	const worldmodel::WorldRepository& repository_;
	TitleBackdrop backdrop_;
	QBasicTimer cloudTimer_;
	QLabel* emptyLabel_ = nullptr;
	QListWidget* worlds_ = nullptr;
	TitleButton* openButton_ = nullptr;

	std::string selectedName() const;
};
