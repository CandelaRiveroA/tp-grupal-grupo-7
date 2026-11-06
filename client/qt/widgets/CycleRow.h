#pragma once

#include <QPixmap>
#include <QString>
#include <QWidget>

class QLabel;

// Fila de apariencia: [◀][nombre sobre el numero][▶]
class CycleRow: public QWidget {
    Q_OBJECT

public:
    CycleRow(const QString& name, const QPixmap& left, const QPixmap& leftHover,
             const QPixmap& right, const QPixmap& rightHover, QWidget* parent = nullptr);

    void setValue(const QString& text);

signals:
    // -1 = anterior, +1 = siguiente.
    void stepRequested(int direction);

private:
    QLabel* value_;
};
