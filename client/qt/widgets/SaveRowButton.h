#pragma once

#include <QAbstractButton>
#include <QString>
#include <cstdint>

class CharacterLayerSource;
class QPainter;
class QPaintEvent;
class QEvent;
class SaveListSprites;
struct SaveSummary;

class SaveRowButton: public QAbstractButton {
    Q_OBJECT

public:
    SaveRowButton(const SaveSummary& save, int rowNumber, const CharacterLayerSource* source,
                  const SaveListSprites& sprites, QWidget* parent = nullptr);

signals:
    void deleteRequested();

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    QString formatGold(std::uint32_t gold) const;
    QString formatPlayTime(std::uint32_t seconds) const;

    const SaveListSprites& sprites_;

    QString name_;
    QString player_;
    QString date_;

    std::uint32_t gold_;
    std::uint32_t seconds_;
    bool multiplayer_;
};
