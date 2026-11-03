#pragma once

#include <QPixmap>
#include <QWidget>

#include <string>

class UsernamePlate: public QWidget {
    Q_OBJECT

public:
    explicit UsernamePlate(const QPixmap& plate, QWidget* parent = nullptr);

    void setUsername(std::string username);

private:
    QPixmap plate_;
    std::string username_;

    void paintEvent(QPaintEvent* event) override;
};
