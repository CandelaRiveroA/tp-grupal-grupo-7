#include "SaveRowButton.h"

#include <QEnterEvent>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QVBoxLayout>

#include "common/model/draft/SaveSummary.h"
#include "qt/services/CharacterLayerSource.h"
#include "qt/theme/SaveListRects.h"
#include "qt/theme/SaveListSprites.h"
#include "qt/widgets/CharacterPreview.h"
#include "qt/widgets/TitleButton.h"

SaveRowButton::SaveRowButton(const SaveSummary& save, int rowNumber,
                             const CharacterLayerSource* source, const SaveListSprites& sprites,
                             QWidget* parent):
        QAbstractButton(parent),
        sprites_(sprites),
        name_(QString::fromStdString(save.gameName))
        ,
        player_(save.players.empty() ? QString() :
                                       QString::fromStdString(save.players.front().name)),
        date_(QString::fromStdString(save.inGameDate)),
        gold_(save.gold),
        seconds_(save.playTimeSeconds),
        connectedPlayers_(save.connectedPlayers),
        maxPlayers_(save.maxPlayers),
        multiplayer_(save.multiplayer) {
    setObjectName("saveRow");
    setAccessibleName(name_);
    setToolTip(name_);
    setFixedSize(SAVE_ROW_W, SAVE_ROW_H);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_Hover);

    auto* line = new QHBoxLayout(this);
    line->setContentsMargins(SAVE_ROW_MARGIN_LEFT, SAVE_ROW_MARGIN_TOP, SAVE_ROW_MARGIN_RIGHT,
                             SAVE_ROW_MARGIN_BOTTOM);
    line->setSpacing(SAVE_ROW_SPACING);

    auto* index = new QLabel(QString("%1.").arg(rowNumber), this);
    index->setObjectName("saveIndex");
    index->setAttribute(Qt::WA_TransparentForMouseEvents);
    line->addWidget(index, 0, Qt::AlignTop);


    auto* portrait = new CharacterPreview(source, this);
    portrait->setAttribute(Qt::WA_TransparentForMouseEvents);
    portrait->setAppearance(save.players.empty() ? CharacterAppearance{} :
                                                   save.players.front().appearance);
    portrait->setFixedSize(SAVE_PORTRAIT_W, SAVE_PORTRAIT_H);
    line->addWidget(portrait, 0, Qt::AlignVCenter);

    auto* left = new QVBoxLayout();
    left->setSpacing(SAVE_ROW_TEXT_GAP);
    if (!player_.isEmpty()) {
        auto* playerLabel = new QLabel(player_, this);
        playerLabel->setObjectName("savePlayer");
        playerLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        left->addWidget(playerLabel);
    }
    if (multiplayer_) {
        auto* playersLabel = new QLabel(formatPlayers(), this);
        playersLabel->setObjectName("savePlayers");
        playersLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        playersLabel->setToolTip("Jugadores conectados ahora");
        left->addWidget(playersLabel);
    }
    left->addStretch(1);
    if (!date_.isEmpty()) {
        auto* dateLabel = new QLabel(date_, this);
        dateLabel->setObjectName("saveDate");
        dateLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        left->addWidget(dateLabel);
    }
    line->addLayout(left);

    line->addStretch(1);
    if (multiplayer_) {
        auto* badge = new QLabel(this);
        badge->setObjectName("saveBadge");
        badge->setPixmap(sprites_.coopBadge());
        badge->setFixedSize(SAVE_COOP_BADGE_W, SAVE_COOP_BADGE_H);
        badge->setAttribute(Qt::WA_TransparentForMouseEvents);
        badge->setAccessibleName("Cooperativa");
        badge->setToolTip("Cooperativa");
        line->addWidget(badge, 0, Qt::AlignVCenter);
        line->addStretch(1);
    }

    auto* right = new QVBoxLayout();
    right->setSpacing(SAVE_ROW_TEXT_GAP);

    right->addStretch(1);
    auto* deleteLine = new QHBoxLayout();
    deleteLine->addStretch(1);
    auto* deleteButton = new TitleButton(sprites_.deleteIcon(), sprites_.deleteIconHover(),
                                         "Eliminar partida", this);
    deleteButton->setObjectName("deleteSaveButton");
    deleteButton->setFixedSize(SAVE_DELETE_SIZE, SAVE_DELETE_SIZE);

    connect(deleteButton, &QAbstractButton::clicked, this, &SaveRowButton::deleteRequested);
    deleteLine->addWidget(deleteButton);
    right->addLayout(deleteLine);

    right->addStretch(1);
    auto* nameLine = new QHBoxLayout();
    nameLine->addStretch(1);
    auto* nameLabel = new QLabel(name_, this);
    nameLabel->setObjectName("saveName");
    nameLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    nameLine->addWidget(nameLabel);
    right->addLayout(nameLine);

    right->addStretch(1);
    auto* statsLine = new QHBoxLayout();
    statsLine->setSpacing(SAVE_ROW_STATS_GAP);
    auto* goldBox = new QHBoxLayout();
    goldBox->setSpacing(SAVE_ROW_ICON_GAP);
    auto* coin = new QLabel(this);
    coin->setObjectName("saveCoinIcon");
    coin->setPixmap(sprites_.coinIcon());
    coin->setFixedSize(SAVE_COIN_W, SAVE_COIN_H);
    coin->setAttribute(Qt::WA_TransparentForMouseEvents);
    goldBox->addWidget(coin);
    auto* goldLabel = new QLabel(formatGold(gold_), this);
    goldLabel->setObjectName("saveGold");
    goldLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    goldBox->addWidget(goldLabel);
    statsLine->addLayout(goldBox);
    auto* timeBox = new QHBoxLayout();
    timeBox->setSpacing(SAVE_ROW_ICON_GAP);
    auto* clock = new QLabel(this);
    clock->setObjectName("saveClockIcon");
    clock->setPixmap(sprites_.clockIcon());
    clock->setFixedSize(SAVE_CLOCK_W, SAVE_CLOCK_H);
    clock->setAttribute(Qt::WA_TransparentForMouseEvents);
    timeBox->addWidget(clock);
    auto* timeLabel = new QLabel(formatPlayTime(seconds_), this);
    timeLabel->setObjectName("savePlayTime");
    timeLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    timeBox->addWidget(timeLabel);
    statsLine->addLayout(timeBox);
    right->addLayout(statsLine);

    line->addLayout(right);
}

void SaveRowButton::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    if (!isEnabled()) {
        painter.setOpacity(SAVE_DISABLED_OPACITY);
        painter.drawPixmap(rect(), sprites_.rowBox());
        return;
    }
    if (underMouse() || hasFocus()) {
        painter.drawPixmap(rect(), sprites_.rowBoxHover());
        return;
    }
    painter.drawPixmap(rect(), sprites_.rowBox());
}

void SaveRowButton::enterEvent(QEnterEvent* event) {
    QAbstractButton::enterEvent(event);
    update();
}

void SaveRowButton::leaveEvent(QEvent* event) {
    QAbstractButton::leaveEvent(event);
    update();
}


QString SaveRowButton::formatGold(std::uint32_t gold) const {
    QString digits = QString::number(gold);
    QString grouped;
    const int separators = digits.size() / SAVE_GOLD_GROUP_DIGITS;
    const int suffix = 1;
    grouped.reserve(digits.size() + separators + suffix);
    for (int i = 0; i < digits.size(); ++i) {
        const int remaining = digits.size() - i;
        if (i > 0 && remaining % SAVE_GOLD_GROUP_DIGITS == 0) {
            grouped.append(QLatin1Char('.'));
        }
        grouped.append(digits.at(i));
    }
    grouped.append(QLatin1Char('o'));
    return grouped;
}

QString SaveRowButton::formatPlayTime(std::uint32_t seconds) const {
    const std::uint32_t hours = seconds / SAVE_SECONDS_PER_HOUR;
    const std::uint32_t minutes = (seconds % SAVE_SECONDS_PER_HOUR) / SAVE_SECONDS_PER_MINUTE;
    return QString("%1:%2").arg(hours).arg(minutes, SAVE_MINUTES_WIDTH, SAVE_DECIMAL_BASE,
                                           QLatin1Char('0'));
}


QString SaveRowButton::formatPlayers() const {
    return QString("%1/%2 jugadores").arg(connectedPlayers_).arg(maxPlayers_);
}
