#pragma once

#include <QString>
#include <QWidget>

class QLabel;
class QSlider;

// Deslizador de apariencia: etiqueta, deslizador y valor.
class SliderRow: public QWidget {
    Q_OBJECT

public:
    SliderRow(const QString& name, const QString& objectName, int maximum,
              QWidget* parent = nullptr);

    void setValue(int value);

signals:
    void valueChanged(int value);

private:
    QSlider* slider_;
    QLabel* value_;
};
