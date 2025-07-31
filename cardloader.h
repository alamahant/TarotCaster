#ifndef CARDLOADER_H
#define CARDLOADER_H

#include <QMap>
#include <QPixmap>
#include <QDir>
#include <QRegularExpression>
#include<QApplication>
#include<QString>
#include <QPainter>

class CardLoader
{
public:
    //CardLoader();
    CardLoader(const QString& path);
    void loadCards();
    QString getCardName(int number) { return cardNames.value(number); }
    QPixmap getCardImage(int number);

    QPixmap getCardBack() const;
    struct CardData {
        int number;
        bool reversed;
    };
    QVector<CardData> getRandomCards(int count, bool allowReversed) const;


private:
    QString cardPath;
    QMap<int, QString> cardNames;    // number -> "The Fool", "The Magician" etc
    QMap<int, QPixmap> cardImages;   // number -> card image
    QString formatName(const QString& rawName);
    QPixmap cardBack;
    QMap<int, QPixmap> scaledCardImages; // Pre-scaled versions for better performance
    QPixmap scaledCardBack; // Pre-scaled back card

    void preScaleCards(); // Method to pre-scale all cards

public:
    void loadDeck(const QString& path);
    const QString& getPath() const { return cardPath; }
private:
    QPixmap loadCrispPixmap(const QString& path);

};

#endif // CARDLOADER_H
