#ifndef TAROTSCENE_H
#define TAROTSCENE_H
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QGraphicsScene>
#include "cardloader.h"
#include "tarotcarditem.h"
#include "customspreaddesigner.h"

class TarotScene : public QGraphicsScene {
    Q_OBJECT
private:
    CardLoader cardLoader;
    QList<TarotCardItem*> displayedCards;
    bool allowReversedCards = false;  // Add this member


public:
    TarotScene(QObject *parent = nullptr);
    ~TarotScene();
    void displayCard(int number, bool reversed, const QPointF& pos, bool beRevealed);
    bool beRevealed;
    void setCardLoader(CardLoader* newCardLoader);
    void setReversedCardsAllowed(bool allowed);
    void displayCelticCross();
    void displayThreeCardSpread();
    void displaySingleCard();
    void displayHorseshoeSpread();
    void displayZodiacSpread();
    const CardLoader& getCardLoader() const { return cardLoader; }


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
        CelticCross,
        ZodiacSpread,
        Custom  // Add this

    };

    struct CustomSpreadPosition {
        int number;
        QString name;
        QString significance;
        qreal x;
        qreal y;
    };

    struct CustomSpread {
        QString name;
        QString description;
        QVector<CustomSpreadPosition> positions;
    };

private:
    SpreadType currentSpreadType = NoSpread;

//save/load
public:
    //void displaySavedSpread(SpreadType type, const QVector<CardLoader::CardData>& savedCards);
    void displaySavedSpread(SpreadType type, const QVector<CardLoader::CardData>& savedCards, const QString& customSpreadName = QString());


private:
    void displaySavedSingleCard(const QVector<CardLoader::CardData>& savedCards);
    void displaySavedThreeCardSpread(const QVector<CardLoader::CardData>& savedCards);
    void displaySavedHorseshoeSpread(const QVector<CardLoader::CardData>& savedCards);
    void displaySavedCelticCross(const QVector<CardLoader::CardData>& savedCards);
    void displaySavedZodiacSpread(const QVector<CardLoader::CardData>& savedCards);

public:
    SpreadType getCurrentSpreadType() const { return currentSpreadType; }
    QVector<CardLoader::CardData> getCurrentCards() const { return currentCards; }

    //custom spread methods
    bool saveCustomSpread(const CustomSpread& spread);
    QVector<CustomSpread> getCustomSpreads() const;
    bool deleteCustomSpread(const QString& spreadName);

    bool displayCustomSpread(const QString& spreadName);
private:
    void displaySavedCustomSpread(const QVector<CardLoader::CardData>& savedCards, const QString& spreadName);

    QString currentCustomSpreadName;

signals:
    void viewRefreshRequested();

};

#endif
