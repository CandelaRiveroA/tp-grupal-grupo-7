#pragma once

#include <QAbstractButton>
#include <QPixmap>
#include <QString>


class TitleButton: public QAbstractButton {
    Q_OBJECT

public:
    TitleButton(const QPixmap& normal, const QPixmap& hover, const QString& accessibleText,
                QWidget* parent = nullptr);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    QPixmap normal_;
    QPixmap hover_;
};
