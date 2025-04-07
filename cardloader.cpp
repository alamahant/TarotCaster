#include "cardloader.h"
#include <QRandomGenerator>
#include "dockcontrols.h"

CardLoader::CardLoader(const QString& path) : cardPath(path) {
}

void CardLoader::loadCards()
{
    QDir dir(cardPath);
    QStringList filters;
    filters << "*.jpg" << "*.png" << "*.jpeg";

    for(const QString& file : dir.entryList(filters)) {
        if (file.length() >= 6 && file[0].isDigit() && file[1].isDigit()) {
            int number = file.left(2).toInt();
            cardImages[number] = QPixmap(dir.filePath(file));
        }
    }

    //QString backPath = cardPath + "/back.png";
    QString backPath = QFile::exists(cardPath + "/back.png") ? cardPath + "/back.png" :
                           (QFile::exists(cardPath + "/Back.png") ? cardPath + "/Back.png" :
                                (QFile::exists(cardPath + "/back.jpg") ? cardPath + "/back.jpg" :
                                     (QFile::exists(cardPath + "/Back.jpg") ? cardPath + "/Back.jpg" :
                                          (QFile::exists(cardPath + "/back.jpeg") ? cardPath + "/back.jpeg" :
                                               cardPath + "/Back.jpeg"))));

    cardBack = QPixmap(backPath);
}


QString CardLoader::formatName(const QString &rawName)
{
    if(rawName.contains("_")) {
        // Handle minor arcana
        QStringList parts = rawName.split("_");
        return QString("%1 of %2").arg(parts.last().at(0).toUpper() + parts.last().mid(1).toLower(),
                                       parts.first().at(0).toUpper() + parts.first().mid(1).toLower());
    } else {
        // Handle major arcana
        return QString("The %1").arg(rawName.at(0).toUpper() + rawName.mid(1).toLower());

    }
}

QPixmap CardLoader::getCardBack() const {
    return cardBack.scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}


QPixmap CardLoader::getCardImage(int number) {
    return cardImages.value(number).scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);

}



QVector<CardLoader::CardData> CardLoader::getRandomCards(int count, bool allowReversed) const {
    QVector<CardData> cards;
    QSet<int> usedCards;

    while (cards.size() < count) {
        int newCard = DockControls::randomGen.bounded(0, 78);

        if (!usedCards.contains(newCard)) {
            bool isReversed = allowReversed && DockControls::randomGen.bounded(2) == 1;
            cards.append({newCard, isReversed});
            usedCards.insert(newCard);
        }
    }
    return cards;
}
