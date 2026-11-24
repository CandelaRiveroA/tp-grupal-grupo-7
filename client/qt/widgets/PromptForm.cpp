#include "PromptForm.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

#include "qt/theme/JoinCodeRects.h"
#include "qt/widgets/InputBox.h"
#include "qt/widgets/ParchmentBanner.h"
#include "qt_theme/TitleButton.h"

PromptForm::PromptForm(const QString& prompt, QWidget* parent)
	: QWidget(parent)
	, sprites_()
	, prompt_(new QLabel(prompt, this))
	, banner_(nullptr)
	, box_(nullptr)
	, ok_(nullptr) {
	auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	banner_ = new ParchmentBanner(sprites_.scrollBanner(), prompt_, this);
	layout->addWidget(banner_, 0, Qt::AlignHCenter);
	layout->addSpacing(JOIN_SCROLL_ROW_GAP);

	auto* row = new QHBoxLayout();
	row->setSpacing(JOIN_INPUT_OK_GAP);
	box_ = new InputBox(
		sprites_.inputBox(),
		QMargins(JOIN_INPUT_PAD_LEFT, JOIN_INPUT_PAD_TOP,
			 JOIN_INPUT_PAD_RIGHT, JOIN_INPUT_PAD_BOTTOM),
		this);
	ok_ = new TitleButton(sprites_.okButton(), sprites_.okButtonHover(), "OK",
			      this);
	row->addStretch();
	row->addWidget(box_);
	row->addWidget(ok_, 0, Qt::AlignVCenter);
	row->addStretch();
	layout->addLayout(row);
}

QLabel* PromptForm::prompt() const { return prompt_; }

QWidget* PromptForm::banner() const { return banner_; }

QWidget* PromptForm::inputBox() const { return box_; }

QLineEdit* PromptForm::edit() const { return box_->edit(); }

TitleButton* PromptForm::okButton() const { return ok_; }
