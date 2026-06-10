#include "tarotscene.h"
#include<QHash>
#include<QVBoxLayout>
#include<QLabel>
#include<QPushButton>
#include<QInputDialog>
#include<QComboBox>
#include<QLineEdit>
#include"Globals.h"

void TarotScene::setCardLoader(CardLoader* newCardLoader)
{
    //cardLoader = *newCardLoader;
    cardLoader.loadDeck(newCardLoader->getPath());

}

void TarotScene::setReversedCardsAllowed(bool allowed)
{

}

TarotScene::TarotScene(QObject *parent) : QGraphicsScene(parent),
    #ifdef GENTOO_BUILD
    cardLoader("/usr/share/tarotcaster/decks/OriginalRiderWaite")
    #else
    cardLoader(QApplication::applicationDirPath() + "/decks/OriginalRiderWaite")
    #endif
{
    setBackgroundBrush(Qt::black);
    cardLoader.loadCards();  // Load initial deck
    //loadCardNames(QApplication::applicationDirPath() + ":resources/card_meanings.json");
    loadCardNames(":/resources/card_meanings.json");


}

TarotScene::~TarotScene() {
    qDeleteAll(displayedCards);
    displayedCards.clear();
    for(auto popup : activePopups){
        delete popup;
    }
    activePopups.clear();
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



void TarotScene::clearScene() {
    for(auto item : items()) {
        removeItem(item);
        delete item;
    }
    emit viewRefreshRequested();
}

void TarotScene::setAllowReversedCards(bool allow) {
    allowReversedCards = allow;
}


void TarotScene::displayThreeCardSpread() {
    clearScene();
    currentSpreadType = ThreeCard;
    readingRequested = false;
    currentCards = cardLoader.getRandomCards(3, allowReversedCards);

    qreal cardWidth = getCardWidth();
    qreal horizontalSpacing = getHorizontalSpacing();

    QPointF center = sceneRect().center();

    // Use the stored cards for display
    displayCard(currentCards[0].number, currentCards[0].reversed,
                QPointF(center.x() - cardWidth - horizontalSpacing, center.y()));
    displayCard(currentCards[1].number, currentCards[1].reversed, center);
    displayCard(currentCards[2].number, currentCards[2].reversed,
                QPointF(center.x() + cardWidth + horizontalSpacing, center.y()));
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



    QPointF center = sceneRect().center();

    // Display single card in center
    displayCard(currentCards[0].number, currentCards[0].reversed, center);

}


void TarotScene::displayFullDeck() {
    clearScene();

    qreal cardWidth = getCardWidth();
    qreal cardHeight = getCardHeight();

    // Calculate how many cards fit based on screen width
    int CARDS_PER_ROW = qMax(6, qMin(10, (int)(sceneRect().width() / (cardWidth * 1.2))));

    // Spacing based on card size (20% of card size for gaps)
    qreal horizontalSpacing = cardWidth * 0.035;
    qreal verticalSpacing = cardHeight * 0.05;

    for(int i = 0; i < 78; i++) {
        int row = i / CARDS_PER_ROW;
        int col = i % CARDS_PER_ROW;

        QPointF pos(col * (cardWidth + horizontalSpacing),
                    row * (cardHeight + verticalSpacing));

        TarotCardItem* card = new TarotCardItem(cardLoader.getCardImage(i),
                                                cardLoader.getCardBack(), i);
        card->setPos(pos);
        card->flip();
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
            currentCustomSpreadName = customSpreadName;
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

    qreal cardWidth = getCardWidth();
    qreal horizontalSpacing = getHorizontalSpacing();
    QPointF center = sceneRect().center();

    displayCard(savedCards[0].number, savedCards[0].reversed,
                QPointF(center.x() - cardWidth - horizontalSpacing, center.y()), beRevealed);
    displayCard(savedCards[1].number, savedCards[1].reversed, center, beRevealed);
    displayCard(savedCards[2].number, savedCards[2].reversed,
                QPointF(center.x() + cardWidth + horizontalSpacing, center.y()), beRevealed);
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


void TarotScene::displaySavedCustomSpread(const QVector<CardLoader::CardData>& savedCards, const QString& spreadName)
{

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

    beRevealed = true;

    // Display saved cards at their positions
    for (int i = 0; i < selectedSpread->positions.size(); ++i) {
        const auto& pos = selectedSpread->positions[i];
        if (i < savedCards.size()) {
            const auto& card = savedCards[i];
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



void TarotScene::redrawCurrentSpread()
{

    if (currentCards.isEmpty()) {
        return;
    }

    // Store current data
    SpreadType currentType = currentSpreadType;
    QString customName = currentCustomSpreadName;
    QVector<CardLoader::CardData> savedCards = currentCards;
    bool wasReadingRequested = readingRequested;


    // Clear and redraw
    clearScene();

    if (currentType == Custom && !customName.isEmpty()) {
        displaySavedCustomSpread(savedCards, customName);
    } else {
        displaySavedSpread(currentType, savedCards, customName);
    }

    readingRequested = wasReadingRequested;
}


QVector<QPointF> TarotScene::getCelticCrossPositions() const {
    QVector<QPointF> positions;

    qreal cardWidth = getCardWidth();
    qreal cardHeight = getCardHeight();
    qreal horizontalSpacing = getHorizontalSpacing() * 0.2;  // 0.8

    // Center-to-center distance = card height + 50% gap (no overlap)
    qreal verticalSpacing = cardHeight * 1.07; // 1.8

    QPointF center = sceneRect().center();

    positions.append(center);  // Center
    positions.append(QPointF(center.x() - (cardWidth * 2.3) - (horizontalSpacing * 1.8), center.y()));  // Crossing
    positions.append(QPointF(center.x(), center.y() + verticalSpacing));  // Below
    positions.append(QPointF(center.x() - cardWidth - horizontalSpacing, center.y()));  // Left
    positions.append(QPointF(center.x(), center.y() - verticalSpacing));  // Above
    positions.append(QPointF(center.x() + cardWidth + horizontalSpacing, center.y()));  // Right

    qreal staffX = center.x() + (cardWidth * 2) + (horizontalSpacing * 2);
    qreal bottomY = center.y() + verticalSpacing;

    for (int i = 0; i < 4; i++) {
        positions.append(QPointF(staffX, bottomY - (i * verticalSpacing)));
    }

    return positions;
}

void TarotScene::displayCelticCross() {
    clearScene();
    currentSpreadType = CelticCross;
    readingRequested = false;

    currentCards = cardLoader.getRandomCards(10, allowReversedCards);

    QVector<QPointF> positions = getCelticCrossPositions();

    for (int i = 0; i < positions.size(); ++i) {
        displayCard(currentCards[i].number, currentCards[i].reversed, positions[i]);
    }
}

void TarotScene::displaySavedCelticCross(const QVector<CardLoader::CardData>& savedCards) {
    if (savedCards.size() < 10) return;
    beRevealed = true;

    QVector<QPointF> positions = getCelticCrossPositions();

    for (int i = 0; i < positions.size(); ++i) {
        displayCard(savedCards[i].number, savedCards[i].reversed, positions[i], beRevealed);
    }
}


QVector<QPointF> TarotScene::getHorseshoePositions() const
{
    QVector<QPointF> positions;

    qreal cardWidth = getCardWidth();
    qreal cardHeight = getCardHeight();
    qreal horizontalSpacing = cardWidth * 1.2;
    qreal verticalSpacing = cardHeight * 1.1;

    QPointF center = sceneRect().center();

    // Left column (1-2-3)
    positions.append(QPointF(center.x() - horizontalSpacing, center.y() + verticalSpacing));
    positions.append(QPointF(center.x() - horizontalSpacing, center.y()));
    positions.append(QPointF(center.x() - horizontalSpacing, center.y() - verticalSpacing));

    // Top center (4)
    positions.append(QPointF(center.x(), center.y() - verticalSpacing * 2));

    // Right column (5-6-7)
    positions.append(QPointF(center.x() + horizontalSpacing, center.y() - verticalSpacing));
    positions.append(QPointF(center.x() + horizontalSpacing, center.y()));
    positions.append(QPointF(center.x() + horizontalSpacing, center.y() + verticalSpacing));

    return positions;
}


QVector<QPointF> TarotScene::getZodiacPositions() const
{
    QVector<QPointF> positions;

    qreal cardWidth = getCardWidth();
    qreal cardHeight = getCardHeight();

    // Keep your original radius values but scale them
    qreal baseRadius = 365 * (cardWidth / 170);  // Scale from original 200px card width
    QPointF center = sceneRect().center();

    for (int i = 0; i < 12; i++) {
        qreal angle = M_PI + (i * 2 * M_PI / 12);
        qreal currentRadius = baseRadius;

        if (i == 0 || i == 6) {
            currentRadius = baseRadius + (190 * (cardWidth / 200));
        }
        if (i == 5 || i == 7 || i == 11 || i == 1) {
            currentRadius = baseRadius + (60 * (cardWidth / 200));
        }

        qreal x = center.x() + currentRadius * cos(angle) - (cardWidth / 2);
        qreal y = center.y() + currentRadius * sin(angle) - (cardHeight / 2);
        positions.append(QPointF(x, y));
    }

    return positions;
}

void TarotScene::displayHorseshoeSpread() {
    clearScene();
    currentSpreadType = Horseshoe;
    readingRequested = false;

    currentCards = cardLoader.getRandomCards(7, allowReversedCards);

    QVector<QPointF> positions = getHorseshoePositions();

    for (int i = 0; i < positions.size(); ++i) {
        displayCard(currentCards[i].number, currentCards[i].reversed, positions[i]);
    }
}

void TarotScene::displaySavedHorseshoeSpread(const QVector<CardLoader::CardData>& savedCards) {
    if (savedCards.size() < 7) return;
    beRevealed = true;

    QVector<QPointF> positions = getHorseshoePositions();

    for (int i = 0; i < positions.size(); ++i) {
        displayCard(savedCards[i].number, savedCards[i].reversed, positions[i], beRevealed);
    }
}

void TarotScene::displayZodiacSpread() {
    clearScene();
    currentSpreadType = ZodiacSpread;
    readingRequested = false;

    currentCards = cardLoader.getRandomCards(12, allowReversedCards);

    QVector<QPointF> positions = getZodiacPositions();

    for (int i = 0; i < positions.size(); ++i) {
        displayCard(currentCards[i].number, currentCards[i].reversed, positions[i]);
    }
}

void TarotScene::displaySavedZodiacSpread(const QVector<CardLoader::CardData>& savedCards) {
    if (savedCards.size() < 12) return;
    beRevealed = true;

    QVector<QPointF> positions = getZodiacPositions();

    for (int i = 0; i < positions.size(); ++i) {
        displayCard(savedCards[i].number, savedCards[i].reversed, positions[i], beRevealed);
    }
}


void TarotScene::showExtraCardPopup(int cardNumber, bool reversed) {
    QWidget* mainWindow = QApplication::activeWindow();
    QDialog* popup = new QDialog(mainWindow);
    popup->setObjectName("extraCardPopup");
    popup->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
    popup->setModal(false);
    activePopups.append(popup);

    connect(popup, &QDialog::finished, this, [this, popup]() {
        activePopups.removeAll(popup);
    });

    // Set initial window title
    QString cardName = getCardName(cardNumber);
    QString orientation = reversed ? "(R)" : QString();
    popup->setWindowTitle("One More Card");

    QVBoxLayout* mainLayout = new QVBoxLayout(popup);
    mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(5, 5, 5, 5);

    // Card image
    QLabel* imageLabel = new QLabel();
    QPixmap cardImage = cardLoader.getCardImage(cardNumber);
    if (reversed) {
        QTransform transform;
        transform.rotate(180);
        cardImage = cardImage.transformed(transform);
    }
    cardImage = cardImage.scaled(getCardWidth(), getCardHeight(), Qt::KeepAspectRatio);
    imageLabel->setPixmap(cardImage);
    mainLayout->addWidget(imageLabel, 0, Qt::AlignCenter);

    // Three buttons in one row
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(0);
    buttonLayout->setContentsMargins(0, 0, 0, 0);

    // ComboBox - shows just number (00, 01, etc.), dropdown shows number + name
    QComboBox* cardSelector = new QComboBox();
    cardSelector->setObjectName("extraCardCombo");
    for (int i = 0; i <= 77; i++) {
        QString cardName2 = getCardName(i);
        QString numberStr = QString("%1").arg(i, 2, 10, QChar('0'));
        QString displayText = QString("%1: %2").arg(numberStr).arg(cardName2);
        cardSelector->addItem(displayText, i);

        // Set tooltip for this item (shown when hovering over it in dropdown)
        cardSelector->setItemData(i, displayText, Qt::ToolTipRole);
    }

    cardSelector->setCurrentIndex(cardNumber);

    cardSelector->setItemText(cardNumber, QString("%1").arg(cardNumber));

    cardSelector->setMaximumWidth(45);
    //cardSelector->setMaximumHeight(16);
    buttonLayout->addWidget(cardSelector, 0, Qt::AlignCenter);
    // Edit Title button
    QPushButton* editTitleButton = new QPushButton("*");
    editTitleButton->setToolTip("Edit Title");
    editTitleButton->setObjectName("editButton");
    //editTitleButton->setMaximumHeight(16);
    buttonLayout->addWidget(editTitleButton);

    // Close button
    QPushButton* closeButton = new QPushButton("Close");
    //closeButton->setMaximumHeight(16);
    closeButton->setObjectName("closeButton");
    buttonLayout->addWidget(closeButton);


    QLabel* cardNameLabel = new QLabel();
    cardNameLabel->setText(QString("%1%2").arg(cardName).arg(orientation));
    cardNameLabel->setAlignment(Qt::AlignCenter);  // Center horizontally
    mainLayout->addWidget(cardNameLabel);

    // Add widgets with vertical center alignment

    mainLayout->addLayout(buttonLayout);

    // Connect signals
    connect(cardSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this, popup, imageLabel, cardSelector, cardNameLabel](int index) {
            if (index < 0) return;
            int newCardNumber = cardSelector->itemData(index).toInt();

            // Update the displayed text to just the number
            QString numberStr = QString("%1").arg(newCardNumber);
            cardSelector->setItemText(index, numberStr);

            // Update image
            QPixmap newImage = cardLoader.getCardImage(newCardNumber);
            newImage = newImage.scaled(getCardWidth(), getCardHeight(), Qt::KeepAspectRatio);
            imageLabel->setPixmap(newImage);

            // Update window title (keep orientation as is)
            QString newCardName = getCardName(newCardNumber);
            cardNameLabel->setText(QString("%1").arg(newCardName));



        });

    connect(editTitleButton, &QPushButton::clicked, [popup]() {
        bool ok;
        QString newTitle = QInputDialog::getText(nullptr, "Edit Title",
            "Enter custom title:", QLineEdit::Normal, popup->windowTitle(), &ok);
        if (ok && !newTitle.isEmpty()) {
            popup->setWindowTitle(newTitle);
        }
    });

    connect(closeButton, &QPushButton::clicked, popup, &QDialog::accept);

    popup->show();
}



