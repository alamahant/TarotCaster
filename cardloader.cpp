#include "cardloader.h"
#include <QRandomGenerator>
#include "dockcontrols.h"
#include<QImageReader>
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

            // Use QImageReader for better quality loading
            QImageReader reader(dir.filePath(file));
            reader.setAutoTransform(true);
            reader.setQuality(100);

            QImage image = reader.read();
            if (!image.isNull()) {
                // For high DPI displays
                qreal dpr = qApp->devicePixelRatio();
                if (dpr > 1.0) {
                    image.setDevicePixelRatio(dpr);
                }

                cardImages[number] = QPixmap::fromImage(image);
            } else {
                qWarning() << "Failed to load card image:" << dir.filePath(file) << reader.errorString();
            }
        }
    }


    //QString backPath = cardPath + "/back.png";
    QString backPath = QFile::exists(cardPath + "/back.png") ? cardPath + "/back.png" :
                           (QFile::exists(cardPath + "/Back.png") ? cardPath + "/Back.png" :
                                (QFile::exists(cardPath + "/back.jpg") ? cardPath + "/back.jpg" :
                                     (QFile::exists(cardPath + "/Back.jpg") ? cardPath + "/Back.jpg" :
                                          (QFile::exists(cardPath + "/back.jpeg") ? cardPath + "/back.jpeg" :
                                               cardPath + "/Back.jpeg"))));

    //cardBack = QPixmap(backPath);
    QImageReader backReader(backPath);
    backReader.setAutoTransform(true);
    backReader.setQuality(100);

    QImage backImage = backReader.read();
    if (!backImage.isNull()) {
        // For high DPI displays
        qreal dpr = qApp->devicePixelRatio();
        if (dpr > 1.0) {
            backImage.setDevicePixelRatio(dpr);
        }

        cardBack = QPixmap::fromImage(backImage);
    } else {
        qWarning() << "Failed to load card back image:" << backPath << backReader.errorString();
    }

    // Pre-cache scaled versions for better performance
    preScaleCards();
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
    //return cardBack.scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    return scaledCardBack;

}


QPixmap CardLoader::getCardImage(int number) {
    //return cardImages.value(number).scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    // Return pre-scaled version if available
    if (scaledCardImages.contains(number)) {
        return scaledCardImages[number];
    }

    // Fall back to scaling on-demand if not pre-cached
    if (cardImages.contains(number)) {
        return cardImages[number].scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    // Return empty pixmap if card not found
    qWarning() << "Card number not found:" << number;
    return QPixmap();

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
void CardLoader::preScaleCards() {
    // Pre-scale all cards for better performance
    for (auto it = cardImages.begin(); it != cardImages.end(); ++it) {
        scaledCardImages[it.key()] = it.value().scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    // Pre-scale back card
    scaledCardBack = cardBack.scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}


void CardLoader::loadDeck(const QString& path)
{
    // Clear existing card images
    cardImages.clear();

    // Set the new path
    cardPath = path;

    // Load cards from the new path
    loadCards();
}
