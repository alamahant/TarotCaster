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

};

#endif // CARDLOADER_H
