#include "dockcontrols.h"

QRandomGenerator DockControls::randomGen;

DockControls::DockControls(QWidget *parent) : QWidget{parent}
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    // Create deck selector group
    QGroupBox* deckGroup = new QGroupBox("Deck Selection", this);
    QVBoxLayout* deckLayout = new QVBoxLayout(deckGroup);
    deckSelector = new QComboBox(this);

    // Load decks from cards directory
    connect(deckSelector, &QComboBox::currentTextChanged,
            this, &DockControls::onDeckSelected);

    loadAvailableDecks();
    /*
    int defaultIndex = deckSelector->findText("OriginalRiderWaite");
    if (defaultIndex >= 0) {
        deckSelector->setCurrentIndex(defaultIndex);
    } else if (deckSelector->count() > 0) {
        deckSelector->setCurrentIndex(0);
    }
    */
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
    shuffleButton->setStyleSheet("QPushButton { background-color: #2a2a2a; color: gold; }");
    dealButton = new QPushButton("Deal Cards", this);
    dealButton->setStyleSheet("QPushButton { background-color: #2a2a2a; color: gold; }");
    clearButton = new QPushButton("Clear Cards", this);
    clearButton->setStyleSheet("QPushButton { background-color: #2a2a2a; color: gold; }");

    //display full deck
    displayFullDeckButton = new QPushButton("Display Full Deck", this);
    displayFullDeckButton->setStyleSheet("QPushButton { background-color: #2a2a2a; color: gold; }");
    layout->addWidget(displayFullDeckButton);

    layout->addWidget(clearButton);
    layout->addWidget(shuffleButton);
    layout->addWidget(dealButton);

    // Add Get Reading button
    getReadingButton = new QPushButton("Get Reading", this);
    getReadingButton->setStyleSheet("QPushButton { background-color: darkgoldenrod; color: black; font-weight: bold; }");
    layout->addWidget(getReadingButton);

    // Add a label for the reading section
    QLabel* readingLabel = new QLabel("Reading Interpretation:", this);
    readingLabel->setStyleSheet("color: gold; font-weight: bold;");
    layout->addWidget(readingLabel);

    // Create the reading display area directly in the main layout
    readingDisplay = new QTextEdit(this);
    readingDisplay->setReadOnly(true);
    readingDisplay->setStyleSheet("background-color: #1a1a1a; color: gold; border: none;");
    readingDisplay->setMinimumHeight(200);
    readingDisplay->setPlaceholderText("Your tarot reading will appear here...");

    // Set the text edit to take all available space
    layout->addWidget(readingDisplay, 1); // The stretch factor of 1 makes it take all available space

    // Connect all signals

    connect(allowReversed, &QCheckBox::toggled,
            this, &DockControls::reversedCardsToggled);
    connect(spreadSelector, &QComboBox::currentTextChanged,
            this, &DockControls::spreadTypeChanged);
    connect(dealButton, &QPushButton::clicked, this, &DockControls::onDealClicked);

    // Connect the Get Reading button
    connect(getReadingButton, &QPushButton::clicked, this, &DockControls::getReadingRequested);

    // Set overall styling
    setStyleSheet("QGroupBox { color: gold; border: 1px solid gold; margin-top: 6px; } "
                  "QGroupBox::title { subcontrol-origin: margin; left: 7px; padding: 0px 5px 0px 5px; }"
                  "QComboBox { background-color: #2a2a2a; color: gold; }");

    //shuffle
    mouseTimer = new QTimer(this);
    mouseTimer->setInterval(50);
    connect(mouseTimer, &QTimer::timeout, this, &DockControls::collectMouseData);
    connect(shuffleButton, &QPushButton::clicked, this, &DockControls::toggleShuffle);

    //full deck
    connect(displayFullDeckButton, &QPushButton::clicked, this, &DockControls::onDisplayFullDeckClicked);
}

void DockControls::onDeckSelected(const QString& deckName) {
    QString deckPath = QApplication::applicationDirPath() + "/decks/" + deckName;

    cardLoader = new CardLoader(deckPath);  // We'll modify this to handle different decks
    emit deckLoaded(cardLoader);
}

void DockControls::onReversedToggled(bool allowed) {
    emit reversedCardsToggled(allowed);
}

void DockControls::onDealClicked() {
    emit dealRequested();
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
    qint64 seed = 0;
    for (const qint64& value : entropyData) {
        seed ^= value;
    }
    randomGen.seed(seed);

}

void DockControls::onDisplayFullDeckClicked() {
    emit displayFullDeckRequested();
}


void DockControls::loadAvailableDecks()
{
    deckSelector->clear();

    QString cardsPath = QCoreApplication::applicationDirPath() + "/decks";
    QDir cardsDir(cardsPath);


    QStringList deckDirs = cardsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    foreach (const QString &deckName, deckDirs) {
        deckSelector->addItem(deckName);
    }


    if (deckSelector->count() > 0) {
        deckSelector->setCurrentIndex(0);
    }
}
