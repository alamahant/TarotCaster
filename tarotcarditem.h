#ifndef TAROTCARDITEM_H
#define TAROTCARDITEM_H

#include <QGraphicsPixmapItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsEffect>
#include<QPropertyAnimation>
#include<QAbstractAnimation>
#include<QTransform>
#include<QGraphicsRotation>
#include <QGraphicsSceneWheelEvent>


class TarotCardItem :  public QObject, public QGraphicsPixmapItem {
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)


public:
    TarotCardItem(const QPixmap &front, const QPixmap &back, int number);
    void flip();

    int getCardNumber() const { return cardNumber; }

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    void wheelEvent(QGraphicsSceneWheelEvent *event) override;
signals:
    void cardRevealed(int cardNumber);

private:
    QPixmap frontImage;
    QPixmap backImage;
    bool isFlipped = true;  // Cards start face down
    int cardNumber;  // Add this
};

#endif
