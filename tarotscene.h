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
    //swapeighteleven
    bool swapEightEleven = false;
private:
    void displaySavedCustomSpread(const QVector<CardLoader::CardData>& savedCards, const QString& spreadName);

    QString currentCustomSpreadName;
    // swap eighteleven names
    QString getCardName(int cardNumber) const;

signals:
    void viewRefreshRequested();
public:
    QMap<QString, QString> tarotNames = {
        // Major Arcana (0-21)
        {"00", "The Fool"},
        {"01", "The Magician"},
        {"02", "The Papess/High Priestess"},
        {"03", "The Empress"},
        {"04", "The Emperor"},
        {"05", "The Pope/Hierophant"},
        {"06", "The Lovers"},
        {"07", "The Chariot"},
        {"08", "Strength"},
        {"09", "The Hermit"},
        {"10", "The Wheel"},
        {"11", "Justice"},
        {"12", "The Hanged Man"},
        {"13", "Death"},
        {"14", "Temperance"},
        {"15", "The Devil"},
        {"16", "The Tower"},
        {"17", "The Star"},
        {"18", "The Moon"},
        {"19", "The Sun"},
        {"20", "Judgement"},
        {"21", "The World"},

        // Wands (22-35)
        {"22", "ace of wands"},
        {"23", "two of wands"},
        {"24", "three of wands"},
        {"25", "four of wands"},
        {"26", "five of wands"},
        {"27", "six of wands"},
        {"28", "seven of wands"},
        {"29", "eight of wands"},
        {"30", "nine of wands"},
        {"31", "ten of wands"},
        {"32", "page of wands"},
        {"33", "knight of wands"},
        {"34", "queen of wands"},
        {"35", "king of wands"},

        // Cups (36-49)
        {"36", "ace of cups"},
        {"37", "two of cups"},
        {"38", "three of cups"},
        {"39", "four of cups"},
        {"40", "five of cups"},
        {"41", "six of cups"},
        {"42", "seven of cups"},
        {"43", "eight of cups"},
        {"44", "nine of cups"},
        {"45", "ten of cups"},
        {"46", "page of cups"},
        {"47", "knight of cups"},
        {"48", "queen of cups"},
        {"49", "king of cups"},

        // Swords (50-63)
        {"50", "ace of swords"},
        {"51", "two of swords"},
        {"52", "three of swords"},
        {"53", "four of swords"},
        {"54", "five of swords"},
        {"55", "six of swords"},
        {"56", "seven of swords"},
        {"57", "eight of swords"},
        {"58", "nine of swords"},
        {"59", "ten of swords"},
        {"60", "page of swords"},
        {"61", "knight of swords"},
        {"62", "queen of swords"},
        {"63", "king of swords"},

        // Coins/Pentacles (64-77)
        {"64", "ace of coins"},
        {"65", "two of coins"},
        {"66", "three of coins"},
        {"67", "four of coins"},
        {"68", "five of coins"},
        {"69", "six of coins"},
        {"70", "seven of coins"},
        {"71", "eight of coins"},
        {"72", "nine of coins"},
        {"73", "ten of coins"},
        {"74", "page of coins"},
        {"75", "knight of coins"},
        {"76", "queen of coins"},
        {"77", "king of coins"}
    };


};

#endif
