#include "tarotscene.h"
#include "mainwindow.h"

void TarotScene::setCardLoader(CardLoader* newCardLoader)
{
    cardLoader = *newCardLoader;
}

void TarotScene::setReversedCardsAllowed(bool allowed)
{

}

TarotScene::TarotScene(QObject *parent) : QGraphicsScene(parent),
    cardLoader(QApplication::applicationDirPath() + "/decks/OriginalRiderWaite")
{
    setBackgroundBrush(Qt::black);
    cardLoader.loadCards();  // Load initial deck
    loadCardNames(QApplication::applicationDirPath() + "/card_meanings.json");
}

TarotScene::~TarotScene() {
    qDeleteAll(displayedCards);
    displayedCards.clear();
}


void TarotScene::displayCard(int number, bool reversed, const QPointF& pos) {
    TarotCardItem* card = new TarotCardItem(cardLoader.getCardImage(number),
                                            cardLoader.getCardBack(), number);
    card->setTransformOriginPoint(card->boundingRect().center());
    if (reversed) {
        card->setRotation(180);
    }
    card->setPos(pos);
    addItem(card);
    connect(card, &TarotCardItem::cardRevealed, this, &TarotScene::onCardRevealed);
}

void TarotScene::onCardRevealed(int cardNumber) {

    emit cardMeaningRequested(cardNumber);  // Forward to MainWindow

}


void TarotScene::displayCelticCross() {
    clearScene();
    currentSpreadType = CelticCross;
    readingRequested = false;
    currentCards = cardLoader.getRandomCards(10, allowReversedCards);

    // Display the cards (existing code)
    const qreal CARD_WIDTH = 150;
    const qreal CARD_HEIGHT = 225;
    const qreal HORIZONTAL_SPACING = 40;
    const qreal VERTICAL_SPACING = CARD_HEIGHT + 100;
    QPointF center = sceneRect().center();

    // 1. Center card (Present)
    displayCard(currentCards[0].number, currentCards[0].reversed, center);
    // 2. Crossing card (Challenge)
    displayCard(currentCards[1].number, currentCards[1].reversed,
                QPointF(center.x() - (CARD_WIDTH * 3) - (HORIZONTAL_SPACING * 3), center.y()));
    // 3. Below (Foundation)
    displayCard(currentCards[2].number, currentCards[2].reversed,
                QPointF(center.x(), center.y() + VERTICAL_SPACING));
    // 4. Left (Past)
    displayCard(currentCards[3].number, currentCards[3].reversed,
                QPointF(center.x() - CARD_WIDTH - HORIZONTAL_SPACING, center.y()));
    // 5. Above (Crown)
    displayCard(currentCards[4].number, currentCards[4].reversed,
                QPointF(center.x(), center.y() - VERTICAL_SPACING));
    // 6. Right (Future)
    displayCard(currentCards[5].number, currentCards[5].reversed,
                QPointF(center.x() + CARD_WIDTH + HORIZONTAL_SPACING, center.y()));

    // Staff positions (right column)
    qreal staffX = center.x() + (CARD_WIDTH * 2) + (HORIZONTAL_SPACING * 2);
    qreal bottomY = center.y() + VERTICAL_SPACING;

    // 7-10. Staff cards (bottom to top)
    for(int i = 0; i < 4; i++) {
        displayCard(currentCards[i+6].number, currentCards[i+6].reversed,
                    QPointF(staffX, bottomY - (i * VERTICAL_SPACING)));
    }
}

void TarotScene::clearScene() {
    for(auto item : items()) {
        removeItem(item);
        delete item;
    }

}
void TarotScene::setAllowReversedCards(bool allow) {
    allowReversedCards = allow;
}

void TarotScene::displayThreeCardSpread() {
    clearScene();
    currentSpreadType = ThreeCard;
    readingRequested = false;
    currentCards = cardLoader.getRandomCards(3, allowReversedCards);

    const qreal CARD_WIDTH = 150;
    const qreal CARD_HEIGHT = 225;
    const qreal HORIZONTAL_SPACING = 40;

    QPointF center = sceneRect().center();

    // Use the stored cards for display
    displayCard(currentCards[0].number, currentCards[0].reversed,
                QPointF(center.x() - CARD_WIDTH - HORIZONTAL_SPACING, center.y()));
    displayCard(currentCards[1].number, currentCards[1].reversed, center);
    displayCard(currentCards[2].number, currentCards[2].reversed,
                QPointF(center.x() + CARD_WIDTH + HORIZONTAL_SPACING, center.y()));

}

void TarotScene::setSpreadType(const QString& type) {
    if (type == "Three Card Spread") {
        displayThreeCardSpread();
    } else if (type == "Celtic Cross") {
        displayCelticCross();
    }
}

void TarotScene::displaySingleCard() {
    clearScene();
    currentSpreadType = SingleCard;
    readingRequested = false;
    currentCards = cardLoader.getRandomCards(1, allowReversedCards);

    const qreal CARD_WIDTH = 150;
    const qreal CARD_HEIGHT = 225;

    QPointF center = sceneRect().center();

    // Display single card in center
    displayCard(currentCards[0].number, currentCards[0].reversed, center);

}

void TarotScene::displayHorseshoeSpread() {
    clearScene();
    currentSpreadType = Horseshoe;
    readingRequested = false;
    currentCards = cardLoader.getRandomCards(7, allowReversedCards);

    const qreal CARD_WIDTH = 150;
    const qreal CARD_HEIGHT = 225;
    const qreal HORIZONTAL_SPACING = 200;
    const qreal VERTICAL_SPACING = 350;    // Increased to 350 for ultimate clarity

    QPointF center = sceneRect().center();

    // Left column (1-2-3)
    displayCard(currentCards[0].number, currentCards[0].reversed,
                QPointF(center.x() - HORIZONTAL_SPACING, center.y() + VERTICAL_SPACING));
    displayCard(currentCards[1].number, currentCards[1].reversed,
                QPointF(center.x() - HORIZONTAL_SPACING, center.y()));
    displayCard(currentCards[2].number, currentCards[2].reversed,
                QPointF(center.x() - HORIZONTAL_SPACING, center.y() - VERTICAL_SPACING));
    displayCard(currentCards[3].number, currentCards[3].reversed,
                QPointF(center.x(), center.y() - VERTICAL_SPACING * 2));
    displayCard(currentCards[4].number, currentCards[4].reversed,
                QPointF(center.x() + HORIZONTAL_SPACING, center.y() - VERTICAL_SPACING));
    displayCard(currentCards[5].number, currentCards[5].reversed,
                QPointF(center.x() + HORIZONTAL_SPACING, center.y()));
    displayCard(currentCards[6].number, currentCards[6].reversed,
                QPointF(center.x() + HORIZONTAL_SPACING, center.y() + VERTICAL_SPACING));
}

void TarotScene::displayFullDeck() {
    clearScene();

    const int CARDS_PER_ROW = 8;  // Reduced from 13
    const qreal CARD_WIDTH = 120;  // Slightly larger cards
    const qreal CARD_HEIGHT = 180;
    const qreal SPACING = 30;

    for(int i = 0; i < 78; i++) {
        int row = i / CARDS_PER_ROW;
        int col = i % CARDS_PER_ROW;

        QPointF pos(col * (CARD_WIDTH + SPACING), row * (CARD_HEIGHT + SPACING));
        TarotCardItem* card = new TarotCardItem(cardLoader.getCardImage(i),
                                                cardLoader.getCardBack(), i);
        card->setPos(pos);
        card->flip();  // Show front face immediately
        addItem(card);
        connect(card, &TarotCardItem::cardRevealed, this, &TarotScene::onCardRevealed);
    }
}

void TarotScene::loadCardNames(const QString& jsonPath) {

    QFile file(jsonPath);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull()) {
            //qWarning() << "Failed to parse JSON data";
            file.close();
            return;
        }


        if (!doc.object().contains("tarot_interpretations")) {
            //qWarning() << "JSON does not contain 'tarot_interpretations' key";
            file.close();
            return;
        }

        QJsonArray cards = doc["tarot_interpretations"].toArray();

        int counter = 0;  // This will map directly to our 0-77 card numbers
        for (const QJsonValue& card : cards) {
            QJsonObject cardObj = card.toObject();
            if (!cardObj.contains("name")) {
                //qWarning() << "Card at index" << counter << "has no 'name' field";
                continue;
            }

            QString name = cardObj["name"].toString();
            cardNames[counter] = name;
            counter++;
        }

        file.close();
    } else {
        qWarning() << "Failed to open card meanings file:" << jsonPath;
        qWarning() << "Error:" << file.errorString();
    }
}

QString TarotScene::generateReadingPrompt() const {
    if (currentCards.isEmpty() || currentSpreadType == NoSpread) {
        return "Error: No cards are currently displayed.";
    }

    QString prompt;
    QStringList positions;

    switch (currentSpreadType) {
    case SingleCard:
        prompt = "Please provide a tarot reading for a Single Card draw:\n\n";
        {
            QString cardName = cardNames.value(currentCards[0].number, "Unknown Card");
            QString orientation = currentCards[0].reversed ? "Reversed" : "Upright";
            prompt += QString("Card: %1 (%2)\n")
                          .arg(cardName)
                          .arg(orientation);
        }
        prompt += "\nPlease interpret this card and provide a reading.";
        break;

    case ThreeCard:
        prompt = "Please provide a tarot reading for a Three Card spread with the following cards:\n\n";
        positions = {
            "Past",
            "Present",
            "Future"
        };

        for (int i = 0; i < currentCards.size() && i < 3; i++) {
            QString cardName = cardNames.value(currentCards[i].number, "Unknown Card");
            QString orientation = currentCards[i].reversed ? "Reversed" : "Upright";

            prompt += QString("Position %1 (%2): %3 (%4)\n")
                          .arg(i + 1)
                          .arg(positions[i])
                          .arg(cardName)
                          .arg(orientation);
        }
        prompt += "\nPlease interpret these cards in the context of a Past-Present-Future spread, explaining how they relate to each other and the querent's situation.";
        break;

    case Horseshoe:
        prompt = "Please provide a tarot reading for a Horseshoe spread with the following cards:\n\n";
        positions = {
            "Past",
            "Present",
            "Hidden Influences",
            "Obstacles",
            "External Influences",
            "Advice",
            "Outcome"
        };

        for (int i = 0; i < currentCards.size() && i < 7; i++) {
            QString cardName = cardNames.value(currentCards[i].number, "Unknown Card");
            QString orientation = currentCards[i].reversed ? "Reversed" : "Upright";

            prompt += QString("Position %1 (%2): %3 (%4)\n")
                          .arg(i + 1)
                          .arg(positions[i])
                          .arg(cardName)
                          .arg(orientation);
        }
        prompt += "\nPlease interpret these cards in the context of a Horseshoe spread, explaining the significance of each position and how the cards interact with each other.";
        break;

    case CelticCross:
        prompt = "Please provide a tarot reading for a Celtic Cross spread with the following cards:\n\n";
        positions = {
            "Present (center of the cross)",
            "Challenge (crossing the present)",
            "Foundation (below)",
            "Past (left)",
            "Crown (above)",
            "Future (right)",
            "Self (bottom of staff)",
            "Environment (second from bottom)",
            "Hopes/Fears (third from bottom)",
            "Outcome (top of staff)"
        };

        for (int i = 0; i < currentCards.size() && i < 10; i++) {
            QString cardName = cardNames.value(currentCards[i].number, "Unknown Card");
            QString orientation = currentCards[i].reversed ? "Reversed" : "Upright";

            prompt += QString("Position %1 (%2): %3 (%4)\n")
                          .arg(i + 1)
                          .arg(positions[i])
                          .arg(cardName)
                          .arg(orientation);
        }
        prompt += "\nPlease interpret these cards in the context of a Celtic Cross spread, explaining the significance of each position and how the cards interact with each other.";
        break;

    default:
        return "Error: Unknown spread type.";
    }

    return prompt;
}

bool TarotScene::getReadingRequested() const
{
    return readingRequested;
}

void TarotScene::setReadingRequested(bool newReadingRequested)
{
    readingRequested = newReadingRequested;
}

//save load

void TarotScene::displaySavedSpread(SpreadType type, const QVector<CardLoader::CardData>& savedCards) {
    clearScene();
    currentCards = savedCards;
    currentSpreadType = type;
    readingRequested = true; // Since we're loading a saved reading

    switch (type) {
    case SingleCard:
        displaySavedSingleCard(savedCards);
        break;
    case ThreeCard:
        displaySavedThreeCardSpread(savedCards);
        break;
    case Horseshoe:
        displaySavedHorseshoeSpread(savedCards);
        break;
    case CelticCross:
        displaySavedCelticCross(savedCards);
        break;
    default:
        // Handle error
        break;
    }
}

void TarotScene::displaySavedSingleCard(const QVector<CardLoader::CardData>& savedCards) {
    if (savedCards.isEmpty()) return;

    QPointF center = sceneRect().center();
    displayCard(savedCards[0].number, savedCards[0].reversed, center);
}

void TarotScene::displaySavedThreeCardSpread(const QVector<CardLoader::CardData>& savedCards) {
    if (savedCards.size() < 3) return;

    const qreal CARD_WIDTH = 150;
    const qreal HORIZONTAL_SPACING = 40;
    QPointF center = sceneRect().center();

    // Past
    displayCard(savedCards[0].number, savedCards[0].reversed,
                QPointF(center.x() - CARD_WIDTH - HORIZONTAL_SPACING, center.y()));
    // Present
    displayCard(savedCards[1].number, savedCards[1].reversed, center);
    // Future
    displayCard(savedCards[2].number, savedCards[2].reversed,
                QPointF(center.x() + CARD_WIDTH + HORIZONTAL_SPACING, center.y()));
}

void TarotScene::displaySavedHorseshoeSpread(const QVector<CardLoader::CardData>& savedCards) {
    if (savedCards.size() < 7) return;

    const qreal CARD_WIDTH = 150;
    const qreal HORIZONTAL_SPACING = 200;
    const qreal VERTICAL_SPACING = 350;
    QPointF center = sceneRect().center();

    // Left column (1-2-3)
    displayCard(savedCards[0].number, savedCards[0].reversed,
                QPointF(center.x() - HORIZONTAL_SPACING, center.y() + VERTICAL_SPACING));
    displayCard(savedCards[1].number, savedCards[1].reversed,
                QPointF(center.x() - HORIZONTAL_SPACING, center.y()));
    displayCard(savedCards[2].number, savedCards[2].reversed,
                QPointF(center.x() - HORIZONTAL_SPACING, center.y() - VERTICAL_SPACING));

    // Top center (4)
    displayCard(savedCards[3].number, savedCards[3].reversed,
                QPointF(center.x(), center.y() - VERTICAL_SPACING * 2));

    // Right column (5-6-7)
    displayCard(savedCards[4].number, savedCards[4].reversed,
                QPointF(center.x() + HORIZONTAL_SPACING, center.y() - VERTICAL_SPACING));
    displayCard(savedCards[5].number, savedCards[5].reversed,
                QPointF(center.x() + HORIZONTAL_SPACING, center.y()));
    displayCard(savedCards[6].number, savedCards[6].reversed,
                QPointF(center.x() + HORIZONTAL_SPACING, center.y() + VERTICAL_SPACING));
}

void TarotScene::displaySavedCelticCross(const QVector<CardLoader::CardData>& savedCards) {
    if (savedCards.size() < 10) return;

    const qreal CARD_WIDTH = 150;
    const qreal CARD_HEIGHT = 225;
    const qreal HORIZONTAL_SPACING = 40;
    const qreal VERTICAL_SPACING = CARD_HEIGHT + 100;
    QPointF center = sceneRect().center();

    // 1. Center card (Present)
    displayCard(savedCards[0].number, savedCards[0].reversed, center);
    // 2. Crossing card (Challenge)
    displayCard(savedCards[1].number, savedCards[1].reversed,
                QPointF(center.x() - (CARD_WIDTH * 3) - (HORIZONTAL_SPACING * 3), center.y()));
    // 3. Below (Foundation)
    displayCard(savedCards[2].number, savedCards[2].reversed,
                QPointF(center.x(), center.y() + VERTICAL_SPACING));
    // 4. Left (Past)
    displayCard(savedCards[3].number, savedCards[3].reversed,
                QPointF(center.x() - CARD_WIDTH - HORIZONTAL_SPACING, center.y()));
    // 5. Above (Crown)
    displayCard(savedCards[4].number, savedCards[4].reversed,
                QPointF(center.x(), center.y() - VERTICAL_SPACING));
    // 6. Right (Future)
    displayCard(savedCards[5].number, savedCards[5].reversed,
                QPointF(center.x() + CARD_WIDTH + HORIZONTAL_SPACING, center.y()));

    // Staff positions (right column)
    qreal staffX = center.x() + (CARD_WIDTH * 2) + (HORIZONTAL_SPACING * 2);
    qreal bottomY = center.y() + VERTICAL_SPACING;

    // 7-10. Staff cards (bottom to top)
    for(int i = 0; i < 4; i++) {
        displayCard(savedCards[i+6].number, savedCards[i+6].reversed,
                    QPointF(staffX, bottomY - (i * VERTICAL_SPACING)));
    }
}
