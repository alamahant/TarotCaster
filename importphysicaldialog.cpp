// importphysicaldialog.cpp
#include "importphysicaldialog.h"
#include "Globals.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include"journalmanager.h"

ImportPhysicalDialog::ImportPhysicalDialog(QWidget *parent)
    : QDialog(parent)
    , m_spreadType(0)
    , m_cardListWidget(nullptr)
    , m_positionListWidget(nullptr)
    , m_reversedCheckBox(nullptr)
    , m_assignButton(nullptr)
    , m_saveButton(nullptr)
    , m_cancelButton(nullptr)
    , m_selectedPositionLabel(nullptr)
    , m_currentSelectedPosition(-1)
{
    
    setupUI();
    populateCardList();
    loadSpreadPositions();
}

ImportPhysicalDialog::~ImportPhysicalDialog()
{
}

void ImportPhysicalDialog::setupUI()
{
    setWindowTitle("Import Physical Cards");
    setMinimumSize(800, 500);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    QHBoxLayout* contentLayout = new QHBoxLayout();
    
    // Left side - Card list (00-77)
    QVBoxLayout* leftLayout = new QVBoxLayout();
    QLabel* cardListLabel = new QLabel("Available Cards (00-77):", this);
    leftLayout->addWidget(cardListLabel);
    m_cardListWidget = new QListWidget(this);
    leftLayout->addWidget(m_cardListWidget);
    contentLayout->addLayout(leftLayout, 1);
    
    // Center - Assignment controls
    QVBoxLayout* centerLayout = new QVBoxLayout();
    QLabel* arrowLabel = new QLabel("→", this);
    arrowLabel->setAlignment(Qt::AlignCenter);
    arrowLabel->setStyleSheet("font-size: 24px; font-weight: bold;");
    centerLayout->addWidget(arrowLabel);
    
    m_assignButton = new QPushButton("Assign Card to Position", this);
    m_assignButton->setEnabled(false);
    centerLayout->addWidget(m_assignButton);
    
    m_reversedCheckBox = new QCheckBox("Reversed", this);
    m_reversedCheckBox->setEnabled(false);
    centerLayout->addWidget(m_reversedCheckBox);
    
    centerLayout->addStretch();
    contentLayout->addLayout(centerLayout);
    
    // Right side - Position list
    QVBoxLayout* rightLayout = new QVBoxLayout();
    QLabel* positionListLabel = new QLabel("Spread Positions:", this);
    rightLayout->addWidget(positionListLabel);
    m_positionListWidget = new QListWidget(this);
    rightLayout->addWidget(m_positionListWidget);
    m_selectedPositionLabel = new QLabel("No position selected", this);
    rightLayout->addWidget(m_selectedPositionLabel);
    contentLayout->addLayout(rightLayout, 1);
    
    mainLayout->addLayout(contentLayout);
    
    // Bottom buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QLabel *instructionsLabel = new QLabel(this);
    QString instructions = "Note: Open this dialog only after selecting the desired Deck and Spread.";
    instructionsLabel->setText(instructions);
    buttonLayout->addStretch();

    buttonLayout->addWidget(instructionsLabel);
    buttonLayout->addStretch();
    m_saveButton = new QPushButton("Save", this);
    m_cancelButton = new QPushButton("Cancel", this);
    buttonLayout->addWidget(m_saveButton);
    buttonLayout->addWidget(m_cancelButton);
    mainLayout->addLayout(buttonLayout);
    
    // Connections
    connect(m_cardListWidget, &QListWidget::itemSelectionChanged, [this]() {
        bool hasCardSelected = m_cardListWidget->currentItem() != nullptr;
        m_assignButton->setEnabled(hasCardSelected && m_currentSelectedPosition >= 0);
    });
    
    connect(m_positionListWidget, &QListWidget::itemSelectionChanged, [this]() {
        int row = m_positionListWidget->currentRow();
        if (row >= 0 && row < m_positionNames.size()) {
            m_currentSelectedPosition = row;
            m_selectedPositionLabel->setText(QString("Selected: %1").arg(m_positionNames[row]));
            m_reversedCheckBox->setEnabled(true);
            
            // Check if this position already has a card assigned
            for (const auto& card : m_assignedCards) {
                if (card.position == row) {
                    int cardNumber = card.number;
                    for (int i = 0; i < m_cardListWidget->count(); i++) {
                        QListWidgetItem* item = m_cardListWidget->item(i);
                        if (item->data(Qt::UserRole).toInt() == cardNumber) {
                            m_cardListWidget->setCurrentItem(item);
                            m_reversedCheckBox->setChecked(card.reversed);
                            break;
                        }
                    }
                    break;
                }
            }
            
            bool hasCardSelected = m_cardListWidget->currentItem() != nullptr;
            m_assignButton->setEnabled(hasCardSelected);
        } else {
            m_currentSelectedPosition = -1;
            m_selectedPositionLabel->setText("No position selected");
            m_reversedCheckBox->setEnabled(false);
            m_assignButton->setEnabled(false);
        }
    });
    
    connect(m_assignButton, &QPushButton::clicked, this, &ImportPhysicalDialog::onAssignClicked);
    connect(m_saveButton, &QPushButton::clicked, this, &ImportPhysicalDialog::onSaveClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void ImportPhysicalDialog::populateCardList()
{
    for (int i = 0; i <= 77; i++) {
        QString key = QString("%1").arg(i, 2, 10, QChar('0'));
        QString cardName = tarotNames.value(key, "Unknown Card");
        QString displayText = QString("%1: %2").arg(key).arg(cardName);
        QListWidgetItem* item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, i);
        m_cardListWidget->addItem(item);
    }
}

void ImportPhysicalDialog::loadSpreadPositions()
{
    // Read from globals
    QString spreadName = g_currentSpreadName;
    bool isCustom = g_isCustomSpread;
    m_deckName = g_currentDeckName;
    

    // DEBUG: Print what we got from globals

    // Default spreads and their positions
    if (!isCustom) {
        if (spreadName == "Single Card") {
            m_positionNames = {"The Card"};
            m_spreadType = 1; // SingleCard
        }
        else if (spreadName == "Three Card") {
            m_positionNames = {"Past", "Present", "Future"};
            m_spreadType = 2; // ThreeCard
        }
        else if (spreadName == "Celtic Cross") {
            m_positionNames = {
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
            m_spreadType = 4; // CelticCross
        }
        else if (spreadName == "Horseshoe") {
            m_positionNames = {
                "Past",
                "Present",
                "Hidden Influences",
                "Obstacles",
                "External Influences",
                "Advice",
                "Outcome"
            };
            m_spreadType = 3; // Horseshoe
        }
        else if (spreadName == "ZodiacSpread") {
            m_positionNames = {
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
            m_spreadType = 5; // ZodiacSpread
        }
        else {
            // Unknown default spread
            showErrorMessage(QString("Unknown spread: '%1'").arg(spreadName));
            reject();
            return;
        }
    }
    else {
        // Custom spread - read from custom_spreads.json
        QString jsonPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/custom_spreads.json";
        
        QFile file(jsonPath);
        if (!file.open(QIODevice::ReadOnly)) {
            showErrorMessage(QString("Cannot open custom_spreads.json - file not found.\nPath: %1").arg(jsonPath));
            reject();
            return;
        }
        
        QByteArray data = file.readAll();
        file.close();
        
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isArray()) {
            showErrorMessage("custom_spreads.json is corrupted (not an array)");
            reject();
            return;
        }
        
        QJsonArray spreadsArray = doc.array();
        bool found = false;
        
        for (const QJsonValue& value : spreadsArray) {
            QJsonObject spreadObj = value.toObject();
            QString name = spreadObj["name"].toString();
            
            if (name == spreadName) {
                // Found the custom spread
                QJsonArray positionsArray = spreadObj["positions"].toArray();
                


                // Extract just the names in order
                m_positionNames.clear();
                for (const QJsonValue& posValue : positionsArray) {
                              QJsonObject posObj = posValue.toObject();
                              QString posName = posObj["name"].toString();
                              m_positionNames.append(posName);
                          }
                
                m_spreadType = 6; // Custom spread has no fixed type ID
                found = true;
                break;
            }
        }
        
        if (!found) {
            showErrorMessage(QString("Cannot find custom spread: '%1' in custom_spreads.json").arg(spreadName));
            reject();
            return;
        }
    }
    
    // Initialize assigned cards array
    m_assignedCards.clear();
    for (int i = 0; i < m_positionNames.size(); i++) {
        AssignedCard card;
        card.number = -1;
        card.position = i;
        card.reversed = false;
        m_assignedCards.append(card);
    }
    
    updatePositionList();
}

void ImportPhysicalDialog::updatePositionList()
{
    if (!m_positionListWidget) return;
    
    m_positionListWidget->clear();
    
    for (int i = 0; i < m_positionNames.size(); i++) {
        QString displayText = QString("Position %1: %2").arg(i).arg(m_positionNames[i]);
        
        // Check if card is assigned
        for (const auto& card : m_assignedCards) {
            if (card.position == i && card.number >= 0) {
                QString cardName = getCardName(card.number);
                QString reversedText = card.reversed ? " (Reversed)" : " (Upright)";
                displayText += QString("\n   → Card %1: %2%3")
                               .arg(card.number, 2, 10, QChar('0'))
                               .arg(cardName)
                               .arg(reversedText);
                break;
            }
        }
        
        m_positionListWidget->addItem(displayText);
    }
}

void ImportPhysicalDialog::onAssignClicked()
{
    if (m_currentSelectedPosition < 0 || m_currentSelectedPosition >= m_positionNames.size()) {
        return;
    }
    
    QListWidgetItem* cardItem = m_cardListWidget->currentItem();
    if (!cardItem) {
        return;
    }
    
    int cardNumber = cardItem->data(Qt::UserRole).toInt();
    bool reversed = m_reversedCheckBox->isChecked();
    

    for (const auto& card : m_assignedCards) {
        if (card.number == cardNumber && card.position != m_currentSelectedPosition) {
            QMessageBox::warning(this, "Duplicate Card",
                QString("Card %1 is already assigned to Position %2.\nRemove it first or choose another card.")
                .arg(cardNumber).arg(card.position));
            return;
        }
    }

    // Update or add assignment
    bool found = false;
    for (int i = 0; i < m_assignedCards.size(); i++) {
        if (m_assignedCards[i].position == m_currentSelectedPosition) {
            m_assignedCards[i].number = cardNumber;
            m_assignedCards[i].reversed = reversed;
            found = true;
            break;
        }
    }
    
    if (!found) {
        AssignedCard newCard;
        newCard.number = cardNumber;
        newCard.position = m_currentSelectedPosition;
        newCard.reversed = reversed;
        m_assignedCards.append(newCard);
    }
    
    updatePositionList();
}

void ImportPhysicalDialog::onSaveClicked()
{
    // Check if all positions have cards assigned
    for (int i = 0; i < m_positionNames.size(); i++) {
        bool found = false;
        for (const auto& card : m_assignedCards) {
            if (card.position == i && card.number >= 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            QMessageBox::warning(this, "Incomplete", 
                QString("Position %1 has no card assigned. Please assign a card to all positions before saving.")
                .arg(i));
            return;
        }
    }
    
    // Build JSON
    QJsonObject json = buildJson();
    
    // Open save dialog
    QString saveDir = getLocalDataDirPath();
    QDir dir;
    if (!dir.exists(saveDir)) {
        dir.mkpath(saveDir);
    }
    
    QString fileName = QFileDialog::getSaveFileName(this, 
        "Save Tarot Reading",
        saveDir + "/imported_spread.tarot",
        "Tarot Files (*.tarot)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    if (!fileName.endsWith(".tarot", Qt::CaseInsensitive)) {
        fileName += ".tarot";
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "Error", "Could not save file: " + file.errorString());
        return;
    }
    
    QJsonDocument doc(json);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();


    // ========== ADD JOURNAL ENTRY ==========
        // Get spread type as string
        QString spreadTypeStr;
        if (g_isCustomSpread) {
            spreadTypeStr = g_currentSpreadName;  // Custom spread name
        } else {
            // Default spread names
            if (g_currentSpreadName == "Single Card") {
                spreadTypeStr = "Single Card";
            } else if (g_currentSpreadName == "Three Card") {
                spreadTypeStr = "Three Card";
            } else if (g_currentSpreadName == "Celtic Cross") {
                spreadTypeStr = "Celtic Cross";
            } else if (g_currentSpreadName == "Horseshoe") {
                spreadTypeStr = "Horseshoe Spread";
            } else if (g_currentSpreadName == "ZodiacSpread") {
                spreadTypeStr = "Zodiac Spread";
            } else {
                spreadTypeStr = "Unknown";
            }
        }

        // Deck name
        QString deckName = m_deckName;

        // Add entry to journal (empty notes, just the reading)
        JournalManager::instance().addEntry(spreadTypeStr, deckName, fileName, "Imported physical spread");
        // =====================================
    
    QMessageBox::information(this, "Success", "Reading saved successfully.");
    accept();
}

QJsonObject ImportPhysicalDialog::buildJson() const
{
    QJsonObject root;
    
    // Cards array
    QJsonArray cardsArray;
    for (const auto& card : m_assignedCards) {
        if (card.number >= 0) {
            QJsonObject cardObj;
            cardObj["number"] = card.number;
            cardObj["position"] = card.position;
            cardObj["reversed"] = card.reversed;
            cardsArray.append(cardObj);
        }
    }
    root["cards"] = cardsArray;
    
    // Deck name (from globals)
    root["deckName"] = m_deckName.isEmpty() ? "Unknown" : m_deckName;
    
    // Query and reading (empty strings as requested)
    root["query"] = "";
    root["reading"] = "";
    
    // Spread type
    root["spreadType"] = m_spreadType;
    // For custom spreads, save the custom spread name
    if (g_isCustomSpread) {
        root["customSpreadName"] = g_currentSpreadName;
    }

    // Timestamp
    root["timestamp"] = getCurrentTimestamp();
    
    // Version
    root["version"] = "1.0";
    
    return root;
}

QString ImportPhysicalDialog::getCurrentTimestamp() const
{
    return QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss");
}

QString ImportPhysicalDialog::getCardName(int number) const
{
    QString key = QString("%1").arg(number, 2, 10, QChar('0'));
    return tarotNames.value(key, "Unknown Card");
}

void ImportPhysicalDialog::showErrorMessage(const QString& message)
{
    QMessageBox::critical(this, "Error", message);
}
