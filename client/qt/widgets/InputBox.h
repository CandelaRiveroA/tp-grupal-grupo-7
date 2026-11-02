#pragma once

#include <QLabel>
#include <QMargins>
#include <QPixmap>

class QLineEdit;

// Caja de texto con marco de sprite: un QLabel que contiene un QLineEdit sin marco.
class InputBox: public QLabel {
public:
    InputBox(const QPixmap& box, const QMargins& padding, QWidget* parent = nullptr);

    QLineEdit* edit() const;

private:
    QLineEdit* edit_;
};
