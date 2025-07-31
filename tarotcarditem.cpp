#include "tarotcarditem.h"

TarotCardItem::TarotCardItem(const QPixmap &front, const QPixmap &back, int number) : QObject(), QGraphicsPixmapItem(back), frontImage(front), backImage(back), cardNumber(number)
{
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsMovable);
    //setFlag(QGraphicsItem::ItemIsSelectable);
    setTransformationMode(Qt::SmoothTransformation);
    //increase crispness
    // Enable high-quality antialiasing for this item
    setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    setCacheMode(QGraphicsItem::ItemCoordinateCache);
    //
    // Enable wheel events
    setFlag(QGraphicsItem::ItemIsFocusable);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);

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

void TarotCardItem::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    // Only handle wheel events when Ctrl is pressed and the card is being hovered
    if (event->modifiers() & Qt::ControlModifier) {
        // Get current scale
        qreal currentScale = scale();

        // Determine zoom factor
        const qreal zoomFactor = 0.15; // 15% change per wheel step

        // Calculate new scale based on wheel direction
        qreal newScale = currentScale;
        if (event->delta() > 0) {
            // Zoom in (up to maximum of 3.0)
            newScale = qMin(currentScale + zoomFactor, 3.0);
        } else {
            // Zoom out (but not below 1.5 while hovering)
            newScale = qMax(currentScale - zoomFactor, 1.5);
        }

        // Apply the new scale
        setScale(newScale);

        // Consume the event
        event->accept();
    } else {
        // Pass the event to the base class if Ctrl is not pressed
        QGraphicsPixmapItem::wheelEvent(event);
    }
}


