#include "dockcontrols.h"
#include<QThread>
QRandomGenerator DockControls::randomGen;


DockControls::DockControls(QWidget *parent)
    : QWidget{parent},
      cardLoader(QApplication::applicationDirPath() + "/decks/OriginalRiderWaite") // Initialize here

{

    generateSeed();

    QVBoxLayout* layout = new QVBoxLayout(this);
    // Create deck selector group
    QGroupBox* deckGroup = new QGroupBox("Deck Selection", this);
    QVBoxLayout* deckLayout = new QVBoxLayout(deckGroup);
    deckSelector = new QComboBox(this);

    // Load decks from cards directory
    connect(deckSelector, &QComboBox::currentTextChanged,
            this, &DockControls::onDeckSelected);

    loadAvailableDecks();
    //
    int index = deckSelector->findText("OriginalRiderWaite");
    if (index >= 0) {
        deckSelector->setCurrentIndex(index);
    }
    //
    if (deckSelector->count() >= 0) {
        onDeckSelected(deckSelector->currentText());
    }
    deckLayout->addWidget(deckSelector);
    layout->addWidget(deckGroup);

    // Create spread selector group
    QGroupBox* spreadGroup = new QGroupBox("Spread Type", this);
    QVBoxLayout* spreadLayout = new QVBoxLayout(spreadGroup);
    spreadSelector = new QComboBox(this);
    spreadSelector->addItem("Single Card");
    spreadSelector->addItem("Three Card");
    spreadSelector->addItem("Celtic Cross");
    spreadSelector->addItem("Horseshoe");
    spreadSelector->addItem("ZodiacSpread");
    //add custom spreads
    if (tarotScene) {
        QVector<TarotScene::CustomSpread> customSpreads = tarotScene->getCustomSpreads();
        for (const auto& spread : customSpreads) {
            spreadSelector->addItem(spread.name, "custom");
        }
    } else {
    }


    //
    spreadLayout->addWidget(spreadSelector);
    layout->addWidget(spreadGroup);

    // Create options group
    QGroupBox* optionsGroup = new QGroupBox("Options", this);
    QVBoxLayout* optionsLayout = new QVBoxLayout(optionsGroup);
    allowReversed = new QCheckBox("Allow Reversed Cards", this);
    optionsLayout->addWidget(allowReversed);
    layout->addWidget(optionsGroup);

    // Create action buttons
    shuffleButton = new QPushButton("Shuffle Deck", this);

    dealButton = new QPushButton("Deal Cards", this);

    clearButton = new QPushButton("Clear Cards", this);


    //display full deck
    displayFullDeckButton = new QPushButton("Display Full Deck", this);

    layout->addWidget(displayFullDeckButton);
    layout->addWidget(clearButton);
    layout->addWidget(shuffleButton);
    layout->addWidget(dealButton);

    // Add Get Reading button
    getReadingButton = new QPushButton("Get Reading", this);
    layout->addWidget(getReadingButton);

    // Create the reading display area directly in the main layout
    readingDisplay = new QTextEdit(this);
    readingDisplay->setReadOnly(true);
    //readingDisplay->setStyleSheet("background-color: #1a1a1a; color: gold; border: none;");
    readingDisplay->setMinimumHeight(200);
    readingDisplay->setPlaceholderText("Your tarot reading will appear here...");

    // Set the text edit to take all available space
    layout->addWidget(readingDisplay, 1); // The stretch factor of 1 makes it take all available space

    shuffleButton->setObjectName("shuffleButton");
    dealButton->setObjectName("dealButton");
    clearButton->setObjectName("clearButton");
    displayFullDeckButton->setObjectName("displayFullDeckButton");
    getReadingButton->setObjectName("readingButton");
    readingDisplay->setObjectName("readingDisplay");

    // Connect all signals

    connect(allowReversed, &QCheckBox::toggled,
            this, &DockControls::reversedCardsToggled);
    connect(spreadSelector, &QComboBox::currentTextChanged,
            this, &DockControls::spreadTypeChanged);
    connect(dealButton, &QPushButton::clicked, this, &DockControls::onDealClicked);

    // Connect the Get Reading button
    connect(getReadingButton, &QPushButton::clicked, this, &DockControls::getReadingRequested);

    //shuffle
    mouseTimer = new QTimer(this);
    mouseTimer->setInterval(50);
    connect(mouseTimer, &QTimer::timeout, this, &DockControls::collectMouseData);
    connect(shuffleButton, &QPushButton::clicked, this, &DockControls::toggleShuffle);

    //full deck
    connect(displayFullDeckButton, &QPushButton::clicked, this, &DockControls::onDisplayFullDeckClicked);
}

DockControls::~DockControls()
{
    // Stop the timer if it's running
    if (mouseTimer && mouseTimer->isActive()) {
        mouseTimer->stop();
    }

    // Restore cursor if we're in shuffling mode
    if (isShuffling) {
        QApplication::restoreOverrideCursor();
    }

    // Qt will handle deleting all child widgets automatically
}


void DockControls::toggleShuffle() {
    if (!isShuffling) {
        // Start shuffling
        entropyData.clear();
        mouseTimer->start();
        shuffleButton->setText("Stop Shuffling");
        isShuffling = true;

        // Change cursor to crosshair (+)
        QApplication::setOverrideCursor(Qt::CrossCursor);

    } else {
        // Stop shuffling
        mouseTimer->stop();
        generateSeed();
        shuffleButton->setText("Shuffle Deck");
        isShuffling = false;

        // Restore default cursor
        QApplication::restoreOverrideCursor();
    }
}

void DockControls::collectMouseData() {
    QPoint mousePos = QCursor::pos();
    qint64 entropy = mousePos.x() * mousePos.y() + QDateTime::currentMSecsSinceEpoch();
    entropyData.append(entropy);
}

void DockControls::generateSeed() {
    // Start with current time in milliseconds for basic randomness
    qint64 seed = QDateTime::currentMSecsSinceEpoch();

    // Add process ID and thread ID for additional uniqueness
    seed ^= (qint64)QCoreApplication::applicationPid();
    seed ^= (qint64)QThread::currentThreadId();

    // Add a truly random value from Qt's secure random generator
    QRandomGenerator secureRandom = QRandomGenerator::securelySeeded();
    seed ^= secureRandom.generate64();

    // Add mouse entropy data if available (from shuffling)
    for (const qint64& value : entropyData) {
        seed ^= value;
    }

    // Seed our main random generator
    randomGen.seed(seed);
}


void DockControls::onDisplayFullDeckClicked() {
    emit displayFullDeckRequested();
}




void DockControls::onDealClicked() {
    emit dealRequested();
}

void DockControls::loadAvailableDecks() {
    deckSelector->clear();

    // System decks location (application directory)
    QString systemDecksPath = QCoreApplication::applicationDirPath() + "/decks";

    // User decks location - use the application name from QCoreApplication
    QString appName = QCoreApplication::applicationName();
    if (appName.isEmpty()) {
        appName = "TarotCaster"; // Fallback if app name isn't set
    }

    QString userDecksPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/decks";


    // Create user decks directory if it doesn't exist
    QDir userDecksDir(userDecksPath);
    if (!userDecksDir.exists()) {
        bool created = userDecksDir.mkpath(".");
    }

    // Load system decks
    QDir systemDecksDir(systemDecksPath);
    QStringList systemDeckDirs = systemDecksDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    // Load user decks
    QStringList userDeckDirs = userDecksDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    // Add system decks to selector
    foreach (const QString &deckName, systemDeckDirs) {
        deckSelector->addItem(deckName, systemDecksPath + "/" + deckName);
    }

    // Add user decks to selector
    foreach (const QString &deckName, userDeckDirs) {
        // Check if this deck name already exists in system decks
        if (systemDeckDirs.contains(deckName)) {
            // Add with a "(User)" suffix to distinguish
            deckSelector->addItem(deckName + " (User)", userDecksPath + "/" + deckName);
        } else {
            deckSelector->addItem(deckName, userDecksPath + "/" + deckName);
        }
    }

    if (deckSelector->count() > 0) {
        deckSelector->setCurrentIndex(0);
    }
}




void DockControls::onDeckSelected(const QString& deckName) {
    // Get the full path from the item data
    int index = deckSelector->currentIndex();
    QString deckPath = deckSelector->itemData(index).toString();

    // If the path is empty (for backward compatibility), use the old method
    if (deckPath.isEmpty()) {
        deckPath = QApplication::applicationDirPath() + "/decks/" + deckName;
    }


    //cardLoader = new CardLoader(deckPath);
    cardLoader = CardLoader(deckPath);

    emit deckLoaded(&cardLoader);
}

