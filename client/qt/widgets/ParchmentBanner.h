#pragma once

#include <QPixmap>
#include <QString>
#include <QWidget>

class QLabel;
class QPaintEvent;


class ParchmentBanner: public QWidget {
public:
    ParchmentBanner(const QPixmap& banner, QLabel* label, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QPixmap banner_;
};
