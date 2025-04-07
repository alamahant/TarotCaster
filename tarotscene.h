#ifndef TAROTSCENE_H
#define TAROTSCENE_H

#include <QGraphicsScene>
#include "cardloader.h"
#include "tarotcarditem.h"
#include "cardloader.h"

class TarotScene : public QGraphicsScene {
    Q_OBJECT
private:
    CardLoader cardLoader;
    QList<TarotCardItem*> displayedCards;
    bool allowReversedCards = false;  // Add this member


public:
    TarotScene(QObject *parent = nullptr);
    ~TarotScene();
    void displayCard(int number, bool reversed, const QPointF& pos);

    void setCardLoader(CardLoader* newCardLoader);
    void setReversedCardsAllowed(bool allowed);
    void displayCelticCross();
    void displayThreeCardSpread();
    void displaySingleCard();
    void displayHorseshoeSpread();


public slots:
    void onCardRevealed(int cardNumber);
    void clearScene();
    void setAllowReversedCards(bool allow);  // Add this slot
    void setSpreadType(const QString& type);
    void displayFullDeck();

signals:
    void cardMeaningRequested(int cardNumber);

private:
    QMap<int, QString> cardNames;
    void loadCardNames(const QString& jsonPath);

signals:
    void readingPromptReady(const QString& prompt);

public:
    QString generateReadingPrompt() const;
    bool getReadingRequested() const;
    void setReadingRequested(bool newReadingRequested);

private:
    QVector<CardLoader::CardData> currentCards;
    bool readingRequested = false;

public:
    enum SpreadType {
        NoSpread,
        SingleCard,
        ThreeCard,
        Horseshoe,
        CelticCross
    };

private:
    SpreadType currentSpreadType = NoSpread;

//save/load
public:
    void displaySavedSpread(SpreadType type, const QVector<CardLoader::CardData>& savedCards);

private:
    void displaySavedSingleCard(const QVector<CardLoader::CardData>& savedCards);
    void displaySavedThreeCardSpread(const QVector<CardLoader::CardData>& savedCards);
    void displaySavedHorseshoeSpread(const QVector<CardLoader::CardData>& savedCards);
    void displaySavedCelticCross(const QVector<CardLoader::CardData>& savedCards);

public:
    SpreadType getCurrentSpreadType() const { return currentSpreadType; }
    QVector<CardLoader::CardData> getCurrentCards() const { return currentCards; }
};

#endif
