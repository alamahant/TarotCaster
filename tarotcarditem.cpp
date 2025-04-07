#include "tarotcarditem.h"

TarotCardItem::TarotCardItem(const QPixmap &front, const QPixmap &back, int number) : QObject(), QGraphicsPixmapItem(back), frontImage(front), backImage(back), cardNumber(number)
{
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsMovable);
    //setFlag(QGraphicsItem::ItemIsSelectable);
}

void TarotCardItem::flip()
{
    emit cardRevealed(cardNumber);

    QPropertyAnimation* fadeAnim = new QPropertyAnimation(this, "opacity");
    fadeAnim->setDuration(300);
    fadeAnim->setStartValue(1.0);
    fadeAnim->setEndValue(0.0);

    connect(fadeAnim, &QPropertyAnimation::finished, [this]() {
        setPixmap(frontImage);  // Always show front image on first click

        QPropertyAnimation* fadeInAnim = new QPropertyAnimation(this, "opacity");
        fadeInAnim->setDuration(300);
        fadeInAnim->setStartValue(0.0);
        fadeInAnim->setEndValue(1.0);
        fadeInAnim->start(QAbstractAnimation::DeleteWhenStopped);
    });

    fadeAnim->start(QAbstractAnimation::DeleteWhenStopped);
}


void TarotCardItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{

    QGraphicsPixmapItem::mousePressEvent(event);
    setZValue(1.0); // Bring to front
    flip();  // Just flip the card
}



void TarotCardItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    setScale(1.5);
    setZValue(100);  // Highest Z-value for hover
    QGraphicsDropShadowEffect* glow = new QGraphicsDropShadowEffect;
    glow->setColor(QColor(255, 215, 0));  // Gold glow
    glow->setBlurRadius(20);
    setGraphicsEffect(glow);
}

void TarotCardItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    setScale(1.0);
    setZValue(0);    // Reset Z-value
    setGraphicsEffect(nullptr);
}

