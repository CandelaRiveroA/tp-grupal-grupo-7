#pragma once

#include <QString>
#include <QWidget>

#include "qt/theme/JoinCodeSprites.h"

class InputBox;
class ParchmentBanner;
class QLabel;
class QLineEdit;
class TitleButton;

// Pergamino + caja + OK
class PromptForm: public QWidget {
public:
	explicit PromptForm(const QString& prompt, QWidget* parent = nullptr);

	QLabel* prompt() const;
	QWidget* banner() const;
	QWidget* inputBox() const;
	QLineEdit* edit() const;
	TitleButton* okButton() const;

private:
	JoinCodeSprites sprites_;
	QLabel* prompt_;
	ParchmentBanner* banner_;
	InputBox* box_;
	TitleButton* ok_;
};
