#include "tarotscene.h"
#include "mainwindow.h"
#include<QHash>

void TarotScene::setCardLoader(CardLoader* newCardLoader)
{
    //cardLoader = *newCardLoader;
    cardLoader.loadDeck(newCardLoader->getPath());

}

void TarotScene::setReversedCardsAllowed(bool allowed)
{

}

TarotScene::TarotScene(QObject *parent) : QGraphicsScene(parent),
    cardLoader(QApplication::applicationDirPath() + "/decks/OriginalRiderWaite")
{
    setBackgroundBrush(Qt::black);
    cardLoader.loadCards();  // Load initial deck
    //loadCardNames(QApplication::applicationDirPath() + ":resources/card_meanings.json");
    loadCardNames(":/resources/card_meanings.json");


}

TarotScene::~TarotScene() {
    qDeleteAll(displayedCards);
    displayedCards.clear();
}


void TarotScene::displayCard(int number, bool reversed, const QPointF& pos, bool beRevealed = false) {
    TarotCardItem* card = new TarotCardItem(cardLoader.getCardImage(number),
                                            cardLoader.getCardBack(), number);
    card->setTransformOriginPoint(card->boundingRect().center());
    if (reversed) {
        card->setRotation(180);
    }
    card->setPos(pos);
    addItem(card);
    connect(card, &TarotCardItem::cardRevealed, this, &TarotScene::onCardRevealed);
    if(beRevealed)
        card->flip();
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
    const qreal HORIZONTAL_SPACING = 55;
    const qreal VERTICAL_SPACING = CARD_HEIGHT + 100;
    QPointF center = sceneRect().center();

    // 1. Center card (Present)
    displayCard(currentCards[0].number, currentCards[0].reversed, center);
    // 2. Crossing card (Challenge)
    displayCard(currentCards[1].number, currentCards[1].reversed,
                QPointF(center.x() - (CARD_WIDTH * 2.3) - (HORIZONTAL_SPACING * 2.3), center.y()));
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

    //emit viewRefreshRequested();
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
    //const qreal CARD_WIDTH = 300;    // 150 * 2
    //const qreal CARD_HEIGHT = 450;   // 225 * 2
    const qreal HORIZONTAL_SPACING = 65; //40

    QPointF center = sceneRect().center();

    // Use the stored cards for display
    displayCard(currentCards[0].number, currentCards[0].reversed,
                QPointF(center.x() - CARD_WIDTH - HORIZONTAL_SPACING, center.y()));
    displayCard(currentCards[1].number, currentCards[1].reversed, center);
    displayCard(currentCards[2].number, currentCards[2].reversed,
                QPointF(center.x() + CARD_WIDTH + HORIZONTAL_SPACING, center.y()));

}

void TarotScene::setSpreadType(const QString& type) {
    /*
    if (type == "Three Card Spread") {
        displayThreeCardSpread();
    } else if (type == "Celtic Cross") {
        displayCelticCross();
    }
*/
}

void TarotScene::displaySingleCard() {
    clearScene();
    currentSpreadType = SingleCard;
    readingRequested = false;
    currentCards = cardLoader.getRandomCards(1, allowReversedCards);

    const qreal CARD_WIDTH = 150; //150
    const qreal CARD_HEIGHT = 225;  //225

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
    const qreal VERTICAL_SPACING = 310;    // Increased to 350 for ultimate clarity

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
    const qreal VERTICAL_SPACING = 110;
    const qreal HORIZONTAL_SPACING = 30;

    //const qreal SPACING = 30;

    for(int i = 0; i < 78; i++) {
        int row = i / CARDS_PER_ROW;
        int col = i % CARDS_PER_ROW;

        //QPointF pos(col * (CARD_WIDTH + SPACING), row * (CARD_HEIGHT + SPACING));
        QPointF pos(col * (CARD_WIDTH + HORIZONTAL_SPACING), row * (CARD_HEIGHT + VERTICAL_SPACING));

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
            //QString cardName = cardNames.value(currentCards[0].number, "Unknown Card");
            QString cardName = getCardName(currentCards[0].number);

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
            //QString cardName = cardNames.value(currentCards[i].number, "Unknown Card");
            QString cardName = getCardName(currentCards[i].number);

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
            //QString cardName = cardNames.value(currentCards[i].number, "Unknown Card");
            QString cardName = getCardName(currentCards[i].number);

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
            //QString cardName = cardNames.value(currentCards[i].number, "Unknown Card");
            QString cardName = getCardName(currentCards[i].number);
            QString orientation = currentCards[i].reversed ? "Reversed" : "Upright";

            prompt += QString("Position %1 (%2): %3 (%4)\n")
                          .arg(i + 1)
                          .arg(positions[i])
                          .arg(cardName)
                          .arg(orientation);
        }
        prompt += "\nPlease interpret these cards in the context of a Celtic Cross spread, explaining the significance of each position and how the cards interact with each other.";
        break;

    case ZodiacSpread:
        prompt = "Please provide a tarot reading for a Zodiac spread with the following cards:\n\n";
        positions = {
            "Aries (1st House): Self, identity, appearance, beginnings",
            "Taurus (2nd House): Possessions, values, resources, self-worth",
            "Gemini (3rd House): Communication, siblings, local travel, learning",
            "Cancer (4th House): Home, family, roots, emotional foundation",
            "Leo (5th House): Creativity, romance, children, pleasure",
            "Virgo (6th House): Health, daily routine, service, work",
            "Libra (7th House): Partnerships, marriage, open enemies, contracts",
            "Scorpio (8th House): Transformation, shared resources, intimacy, death/rebirth",
            "Sagittarius (9th House): Higher education, philosophy, travel, expansion",
            "Capricorn (10th House): Career, public image, authority, achievement",
            "Aquarius (11th House): Friends, groups, hopes, humanitarian pursuits",
            "Pisces (12th House): Unconscious, secrets, spirituality, self-undoing"
        };

        for (int i = 0; i < currentCards.size() && i < 12; i++) {
            //QString cardName = cardNames.value(currentCards[i].number, "Unknown Card");
            QString cardName = getCardName(currentCards[i].number);

            QString orientation = currentCards[i].reversed ? "Reversed" : "Upright";
            prompt += QString("%1: %2 (%3)\n")
                          .arg(positions[i])
                          .arg(cardName)
                          .arg(orientation);
        }

        prompt += "\nPlease interpret these cards in the context of a Zodiac spread, explaining how each card influences the corresponding house in the querent's life. Consider both the zodiac sign's energy and the house's domain when interpreting each position.";
        break;

    case Custom:
    {

        // Get all custom spreads
        QVector<CustomSpread> spreads = getCustomSpreads();

        // Find the current custom spread by name
        const CustomSpread* selectedSpread = nullptr;
        for (const auto& spread : spreads) {
            if (spread.name == currentCustomSpreadName) {
                selectedSpread = &spread;
                break;
            }
        }

        if (selectedSpread && !selectedSpread->positions.isEmpty()) {
            // We found a valid spread

            prompt = QString("Please provide a tarot reading for a '%1' spread with the following cards:\n\n")
                         .arg(selectedSpread->name);

            // Add description if available
            if (!selectedSpread->description.isEmpty()) {
                prompt += "Spread Description: " + selectedSpread->description + "\n\n";
            }

            // Add each card with its position information
            for (int i = 0; i < currentCards.size() && i < selectedSpread->positions.size(); i++) {
                //QString cardName = cardNames.value(currentCards[i].number, "Unknown Card");
                QString cardName = getCardName(currentCards[i].number);

                QString orientation = currentCards[i].reversed ? "Reversed" : "Upright";

                // Get position details
                const CustomSpreadPosition& pos = selectedSpread->positions[i];

                prompt += QString("Position %1 (%2): %3 (%4)\n")
                              .arg(pos.number)
                              .arg(pos.name)
                              .arg(cardName)
                              .arg(orientation);

                // Add significance if available
                if (!pos.significance.isEmpty()) {
                    prompt += QString("   Significance: %1\n").arg(pos.significance);
                }
            }

            prompt += "\nPlease interpret these cards in the context of the '" + selectedSpread->name +
                      "' spread, explaining the significance of each position and how the cards interact with each other.";

        } else {

            // No valid spread found - use a generic prompt
            prompt = "Please provide a tarot reading for a Custom spread with the following cards:\n\n";

            for (int i = 0; i < currentCards.size(); i++) {
                //QString cardName = cardNames.value(currentCards[i].number, "Unknown Card");
                QString cardName = getCardName(currentCards[i].number);

                QString orientation = currentCards[i].reversed ? "Reversed" : "Upright";

                prompt += QString("Position %1: %2 (%3)\n")
                              .arg(i + 1)
                              .arg(cardName)
                              .arg(orientation);
            }

            prompt += "\nPlease interpret these cards in the context of this custom spread, explaining how they relate to each other and the querent's situation.";
        }
    }
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


void TarotScene::displaySavedSpread(SpreadType type, const QVector<CardLoader::CardData>& savedCards, const QString& customSpreadName) {
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
    case ZodiacSpread:
        displaySavedZodiacSpread(savedCards);
        break;
    case Custom:
        // For custom spreads, we need the name to load the layout
        if (!customSpreadName.isEmpty()) {
            displaySavedCustomSpread(savedCards, customSpreadName);

        } else {
            // Handle error - missing custom spread name
            qWarning() << "Cannot display saved custom spread: missing spread name";
        }
        break;
    default:
        // Handle error
        qWarning() << "Unknown spread type:" << type;
        break;
    }
}




void TarotScene::displaySavedSingleCard(const QVector<CardLoader::CardData>& savedCards) {
    if (savedCards.isEmpty()) return;
    beRevealed = true;
    QPointF center = sceneRect().center();
    displayCard(savedCards[0].number, savedCards[0].reversed, center, beRevealed);
}

void TarotScene::displaySavedThreeCardSpread(const QVector<CardLoader::CardData>& savedCards) {
    if (savedCards.size() < 3) return;
    beRevealed = true;
    const qreal CARD_WIDTH = 150;
    const qreal HORIZONTAL_SPACING = 40;
    QPointF center = sceneRect().center();

    // Past
    displayCard(savedCards[0].number, savedCards[0].reversed,
                QPointF(center.x() - CARD_WIDTH - HORIZONTAL_SPACING, center.y()), beRevealed);
    // Present
    displayCard(savedCards[1].number, savedCards[1].reversed, center, beRevealed);
    // Future
    displayCard(savedCards[2].number, savedCards[2].reversed,
                QPointF(center.x() + CARD_WIDTH + HORIZONTAL_SPACING, center.y()), beRevealed);
}

void TarotScene::displaySavedHorseshoeSpread(const QVector<CardLoader::CardData>& savedCards) {
    if (savedCards.size() < 7) return;
    beRevealed = true;
    const qreal CARD_WIDTH = 150;
    const qreal HORIZONTAL_SPACING = 200;
    const qreal VERTICAL_SPACING = 350;
    QPointF center = sceneRect().center();

    // Left column (1-2-3)
    displayCard(savedCards[0].number, savedCards[0].reversed,
                QPointF(center.x() - HORIZONTAL_SPACING, center.y() + VERTICAL_SPACING), beRevealed);
    displayCard(savedCards[1].number, savedCards[1].reversed,
                QPointF(center.x() - HORIZONTAL_SPACING, center.y()), beRevealed);
    displayCard(savedCards[2].number, savedCards[2].reversed,
                QPointF(center.x() - HORIZONTAL_SPACING, center.y() - VERTICAL_SPACING), beRevealed);

    // Top center (4)
    displayCard(savedCards[3].number, savedCards[3].reversed,
                QPointF(center.x(), center.y() - VERTICAL_SPACING * 2), beRevealed);

    // Right column (5-6-7)
    displayCard(savedCards[4].number, savedCards[4].reversed,
                QPointF(center.x() + HORIZONTAL_SPACING, center.y() - VERTICAL_SPACING), beRevealed);
    displayCard(savedCards[5].number, savedCards[5].reversed,
                QPointF(center.x() + HORIZONTAL_SPACING, center.y()), beRevealed);
    displayCard(savedCards[6].number, savedCards[6].reversed,
                QPointF(center.x() + HORIZONTAL_SPACING, center.y() + VERTICAL_SPACING), beRevealed);
}

void TarotScene::displaySavedCelticCross(const QVector<CardLoader::CardData>& savedCards) {
    if (savedCards.size() < 10) return;
    beRevealed = true;
    const qreal CARD_WIDTH = 150;
    const qreal CARD_HEIGHT = 225;
    const qreal HORIZONTAL_SPACING = 40;
    const qreal VERTICAL_SPACING = CARD_HEIGHT + 100;
    QPointF center = sceneRect().center();

    // 1. Center card (Present)
    displayCard(savedCards[0].number, savedCards[0].reversed, center, beRevealed);
    // 2. Crossing card (Challenge)
    displayCard(savedCards[1].number, savedCards[1].reversed,
                QPointF(center.x() - (CARD_WIDTH * 3) - (HORIZONTAL_SPACING * 3), center.y()), beRevealed);
    // 3. Below (Foundation)
    displayCard(savedCards[2].number, savedCards[2].reversed,
                QPointF(center.x(), center.y() + VERTICAL_SPACING), beRevealed);
    // 4. Left (Past)
    displayCard(savedCards[3].number, savedCards[3].reversed,
                QPointF(center.x() - CARD_WIDTH - HORIZONTAL_SPACING, center.y()), beRevealed);
    // 5. Above (Crown)
    displayCard(savedCards[4].number, savedCards[4].reversed,
                QPointF(center.x(), center.y() - VERTICAL_SPACING), beRevealed);
    // 6. Right (Future)
    displayCard(savedCards[5].number, savedCards[5].reversed,
                QPointF(center.x() + CARD_WIDTH + HORIZONTAL_SPACING, center.y()), beRevealed);

    // Staff positions (right column)
    qreal staffX = center.x() + (CARD_WIDTH * 2) + (HORIZONTAL_SPACING * 2);
    qreal bottomY = center.y() + VERTICAL_SPACING;

    // 7-10. Staff cards (bottom to top)
    for(int i = 0; i < 4; i++) {
        displayCard(savedCards[i+6].number, savedCards[i+6].reversed,
                    QPointF(staffX, bottomY - (i * VERTICAL_SPACING)), beRevealed);
    }
}


void TarotScene::displayZodiacSpread() {
    clearScene();
    currentSpreadType = ZodiacSpread;
    readingRequested = false;

    // Zodiac spread uses 12 cards
    currentCards = cardLoader.getRandomCards(12, allowReversedCards);

    // Display constants
    const qreal CARD_WIDTH = 150;
    const qreal CARD_HEIGHT = 225;

    QPointF center = sceneRect().center();

    // Base radius for most positions
    const qreal BASE_RADIUS = 365;

    // Place cards in a circle, starting from 9 o'clock position (Aries)
    // and moving COUNTERCLOCKWISE through the zodiac signs
    for (int i = 0; i < 12; i++) {
        // Calculate angle in radians (starting from 9 o'clock, moving COUNTERCLOCKWISE)
        qreal angle = M_PI + (i * 2 * M_PI / 12); // Changed from - to +

        // Use larger radius for certain positions
        qreal currentRadius = BASE_RADIUS;
        if (i == 0 || i == 6) {
            currentRadius = BASE_RADIUS + 190; // Larger radius for Aries and Libra
        }
        if (i == 5 || i == 7 || i == 11 || i == 1) {
            currentRadius = BASE_RADIUS + 60; // Slightly larger radius for Virgo, Scorpio, Taurus, Pisces
        }

        // Calculate position on the circle
        qreal x = center.x() + currentRadius * cos(angle) - (CARD_WIDTH / 2);
        qreal y = center.y() + currentRadius * sin(angle) - (CARD_HEIGHT / 2);

        // Display the card
        displayCard(currentCards[i].number, currentCards[i].reversed, QPointF(x, y));
    }
}


void TarotScene::displaySavedZodiacSpread(const QVector<CardLoader::CardData>& savedCards) {
    if (savedCards.size() < 12) return;
    beRevealed = true;
    clearScene();
    currentSpreadType = ZodiacSpread;
    readingRequested = false;

    // Display constants
    const qreal CARD_WIDTH = 150;
    const qreal CARD_HEIGHT = 225;

    QPointF center = sceneRect().center();

    // Base radius for most positions
    const qreal BASE_RADIUS = 365;

    // Place cards in a circle, starting from 9 o'clock position (Aries)
    // and moving COUNTERCLOCKWISE through the zodiac signs
    for (int i = 0; i < 12; i++) {
        // Calculate angle in radians (starting from 9 o'clock, moving COUNTERCLOCKWISE)
        qreal angle = M_PI + (i * 2 * M_PI / 12);

        // Use larger radius for certain positions
        qreal currentRadius = BASE_RADIUS;
        if (i == 0 || i == 6) {
            currentRadius = BASE_RADIUS + 190; // Larger radius for Aries and Libra
        }
        if (i == 5 || i == 7 || i == 11 || i == 1) {
            currentRadius = BASE_RADIUS + 60; // Slightly larger radius for Virgo, Scorpio, Taurus, Pisces
        }

        // Calculate position on the circle
        qreal x = center.x() + currentRadius * cos(angle) - (CARD_WIDTH / 2);
        qreal y = center.y() + currentRadius * sin(angle) - (CARD_HEIGHT / 2);

        // Display the saved card
        displayCard(savedCards[i].number, savedCards[i].reversed, QPointF(x, y), beRevealed);
    }
}

bool TarotScene::saveCustomSpread(const TarotScene::CustomSpread& spread)
{

    // Load existing spreads
    QVector<CustomSpread> spreads = getCustomSpreads();

    // Check if a spread with this name already exists
    bool updated = false;
    for (int i = 0; i < spreads.size(); ++i) {
        if (spreads[i].name == spread.name) {
            // Update existing spread
            spreads[i] = spread;
            updated = true;
            break;
        }
    }

    if (!updated) {
        // Add new spread
        spreads.append(spread);
    }

    // Save all spreads to file
    QFile file(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/custom_spreads.json");
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open custom spreads file for writing:" << file.errorString();
        return false;
    }

    QJsonArray spreadsArray;
    for (const auto& s : spreads) {
        QJsonObject spreadObj;
        spreadObj["name"] = s.name;
        spreadObj["description"] = s.description;

        QJsonArray positionsArray;
        for (const auto& p : s.positions) {
            QJsonObject posObj;
            posObj["number"] = p.number;
            posObj["name"] = p.name;
            posObj["significance"] = p.significance;
            posObj["x"] = p.x;
            posObj["y"] = p.y;
            positionsArray.append(posObj);
        }

        spreadObj["positions"] = positionsArray;
        spreadsArray.append(spreadObj);
    }

    QJsonDocument doc(spreadsArray);
    if (file.write(doc.toJson()) == -1) {
        qWarning() << "Failed to write custom spreads data:" << file.errorString();
        return false;
    }

    return true;
}


QVector<TarotScene::CustomSpread> TarotScene::getCustomSpreads() const
{
    QVector<CustomSpread> spreads;

    // Load spreads from file
    QFile file(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/custom_spreads.json");
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return spreads;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isArray()) {
        QJsonArray spreadsArray = doc.array();
        for (const auto& spreadValue : spreadsArray) {
            if (spreadValue.isObject()) {
                QJsonObject spreadObj = spreadValue.toObject();

                CustomSpread spread;
                spread.name = spreadObj["name"].toString();
                spread.description = spreadObj["description"].toString();

                QJsonArray positionsArray = spreadObj["positions"].toArray();
                for (const auto& posValue : positionsArray) {
                    if (posValue.isObject()) {
                        QJsonObject posObj = posValue.toObject();

                        CustomSpreadPosition pos;
                        pos.number = posObj["number"].toInt();
                        pos.name = posObj["name"].toString();
                        pos.significance = posObj["significance"].toString();
                        pos.x = posObj["x"].toDouble();
                        pos.y = posObj["y"].toDouble();

                        spread.positions.append(pos);
                    }
                }

                spreads.append(spread);
            }
        }
    }

    return spreads;
}

bool TarotScene::displayCustomSpread(const QString& spreadName)
{

    // Load custom spreads
    QVector<CustomSpread> spreads = getCustomSpreads();

    // Find the requested spread
    const CustomSpread* selectedSpread = nullptr;
    for (const auto& spread : spreads) {
        if (spread.name == spreadName) {
            selectedSpread = &spread;
            break;
        }
    }

    if (!selectedSpread) {
        qWarning() << "Custom spread not found:" << spreadName;
        return false;
    }

    // Clear the scene
    clearScene();

    // Set current spread type
    currentCustomSpreadName = spreadName;

    currentSpreadType = Custom;
    readingRequested = false;

    // Get random cards
    currentCards = cardLoader.getRandomCards(selectedSpread->positions.size(), allowReversedCards);

    // Display cards at their positions
    for (int i = 0; i < selectedSpread->positions.size(); ++i) {
        const auto& pos = selectedSpread->positions[i];

        // Make sure we have enough cards
        if (i < currentCards.size()) {
            // Display the card
            QPointF position(pos.x, pos.y);
            displayCard(currentCards[i].number,
                        currentCards[i].reversed,
                        position,
                        false);  // Not revealed initially

            // Find the card we just added to set data on it
            for (auto item : items()) {
                if (auto cardItem = dynamic_cast<TarotCardItem*>(item)) {
                    if (cardItem->getCardNumber() == currentCards[i].number) {
                        // Set position data
                        cardItem->setData(1, pos.number);
                        cardItem->setData(2, pos.name);
                        cardItem->setData(3, pos.significance);
                        break;
                    }
                }
            }
        }
    }

    return true;
}

void TarotScene::displaySavedCustomSpread(const QVector<CardLoader::CardData>& savedCards, const QString& spreadName) {

    // Load the custom spread layout
    QVector<CustomSpread> spreads = getCustomSpreads();
    const CustomSpread* selectedSpread = nullptr;

    // Find the requested spread
    for (const auto& spread : spreads) {
        if (spread.name == spreadName) {
            selectedSpread = &spread;
            break;
        }
    }

    if (!selectedSpread) {
        qWarning() << "Custom spread not found:" << spreadName;
        return;
    }

    // Make sure we have enough cards
    if (savedCards.size() < selectedSpread->positions.size()) {
        qWarning() << "Not enough saved cards for this spread";
        return;
    }

    beRevealed = true; // Cards in saved readings are always revealed

    // Display saved cards at their positions
    for (int i = 0; i < selectedSpread->positions.size(); ++i) {
        const auto& pos = selectedSpread->positions[i];

        // Make sure we don't go out of bounds
        if (i < savedCards.size()) {
            const auto& card = savedCards[i];

            // Display the card at its position
            QPointF position(pos.x, pos.y);
            displayCard(card.number, card.reversed, position, beRevealed);

        }
    }

}


bool TarotScene::deleteCustomSpread(const QString& spreadName)
{
    // Load all spreads
    QVector<CustomSpread> spreads = getCustomSpreads();

    // Find and remove the spread with the given name
    bool found = false;
    for (int i = 0; i < spreads.size(); i++) {
        if (spreads[i].name == spreadName) {
            spreads.remove(i);
            found = true;
            break;
        }
    }

    if (!found) {
        return false;
    }

    // Save the updated spreads list
    QFile file(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/custom_spreads.json");
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonArray spreadsArray;
    for (const auto& spread : spreads) {
        QJsonObject spreadObj;
        spreadObj["name"] = spread.name;
        spreadObj["description"] = spread.description;

        QJsonArray positionsArray;
        for (const auto& pos : spread.positions) {
            QJsonObject posObj;
            posObj["number"] = pos.number;
            posObj["name"] = pos.name;
            posObj["significance"] = pos.significance;
            posObj["x"] = pos.x;
            posObj["y"] = pos.y;
            positionsArray.append(posObj);
        }

        spreadObj["positions"] = positionsArray;
        spreadsArray.append(spreadObj);
    }

    QJsonDocument doc(spreadsArray);
    file.write(doc.toJson());

    return true;
}


QString TarotScene::getCardName(int cardNumber) const
{
    // Default lookup
    QString name = cardNames.value(cardNumber, "Unknown Card");

    // Apply swaps if enabled
    if (swapEightEleven) {
        switch(cardNumber) {
        case 11:  return "Strength";
        case 8: return "Justice";
        default: return name;
        }
    }
    return name;

}

