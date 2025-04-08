#include "mainwindow.h"
#include<QMetaMethod>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)

{
    this->setMinimumSize(1200, 800);
    this->showMaximized();

    setWindowTitle("TarotCaster");

    tarotScene = new TarotScene(this);
    centralView = new QGraphicsView(tarotScene);  // Set scene immediately
    centralView->setBackgroundBrush(Qt::black);
    centralView->setRenderHint(QPainter::Antialiasing);
    setCentralWidget(centralView);

    createDocks();

    QString jsonPath = QDir(QCoreApplication::applicationDirPath()).filePath("card_meanings.json");
    cardMeanings = CardMeaning::loadFromJson(jsonPath);

    connect(tarotScene, SIGNAL(cardMeaningRequested(int)),
            this, SLOT(showCardMeaning(int)));


    connect(dockControls->dealButton, &QPushButton::clicked,
            this, &MainWindow::onDealClicked);

    connect(dockControls->clearButton, &QPushButton::clicked,
            tarotScene, &TarotScene::clearScene);

    connect(dockControls->clearButton, &QPushButton::clicked,
            this, &MainWindow::clearMeaningDisplay);

    connect(dockControls->allowReversed, &QCheckBox::toggled,
           tarotScene, &TarotScene::setAllowReversedCards);
    setStyleSheet("background-color: black; color: gold;");

    connect(dockControls, &DockControls::displayFullDeckRequested,
            tarotScene, &TarotScene::displayFullDeck);

    mistralApi = new MistralAPI(this);

    connect(mistralApi, &MistralAPI::readingReady, this, &MainWindow::onReadingReady);

    connect(mistralApi, &MistralAPI::errorOccurred, this, &MainWindow::onApiError);

    // Connect the Get Reading button
    connect(dockControls->getReadingButton, &QPushButton::clicked,
            this, &MainWindow::onGetReadingClicked);

    QMenu* fileMenu = menuBar()->addMenu("&File");
    QAction* saveAction = fileMenu->addAction("&Save Reading", this, &MainWindow::onSaveReading);
    saveAction->setShortcut(QKeySequence::Save);
    QAction* loadAction = fileMenu->addAction("&Load Reading", this, &MainWindow::onLoadReading);
    loadAction->setShortcut(QKeySequence::Open);
    fileMenu->addSeparator();
    fileMenu->addAction("&Exit", this, &QWidget::close, QKeySequence::Quit);

    QMenu *editMenu = menuBar()->addMenu("&Edit");
    editMenu->addAction("&Mistral API Key", this, &MainWindow::onEditApiKey);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI() {
    centralView = new QGraphicsView(this);
    centralView->setBackgroundBrush(Qt::black);
    centralView->setRenderHint(QPainter::Antialiasing);
    setCentralWidget(centralView);
}

void MainWindow::createDocks() {
    leftDock = new QDockWidget("Deck Controls", this);
    dockControls = new DockControls(this);
    leftDock->setWidget(dockControls);
    leftDock->setAllowedAreas(Qt::LeftDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, leftDock);

    rightDock = new QDockWidget("Card Meaning", this);
    meaningDisplay = new MeaningDisplay(this);
    rightDock->setWidget(meaningDisplay);
    rightDock->setAllowedAreas(Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, rightDock);

    // Connect the new signals
    connect(dockControls, &DockControls::deckLoaded,
            this, &MainWindow::onDeckLoaded);
    connect(dockControls, &DockControls::reversedCardsToggled,
            this, &MainWindow::onReversedCardsToggled);
}

void MainWindow::testCards() {

}

void MainWindow::clearMeaningDisplay()
{
    meaningDisplay->clear();
    dockControls->readingDisplay->clear();
}

void MainWindow::showCardMeaning(int cardNumber) {
    if (cardMeanings.contains(cardNumber)) {
        meaningDisplay->displayMeaning(cardMeanings[cardNumber]);
    }
}

void MainWindow::onDeckLoaded(CardLoader* loader) {
    loader->loadCards();  // Load the new deck
    tarotScene->setCardLoader(loader);
    //testCards();
}

void MainWindow::onReversedCardsToggled(bool allowed) {
    // Enable/disable reversed cards in the scene
    tarotScene->setReversedCardsAllowed(allowed);
}

void MainWindow::onDealClicked() {
    QString currentSpread = dockControls->spreadSelector->currentText();
    if (currentSpread == "Single Card") {
        tarotScene->displaySingleCard();
    } else if (currentSpread == "Three Card") {
        tarotScene->displayThreeCardSpread();
    } else if (currentSpread == "Horseshoe") {
        tarotScene->displayHorseshoeSpread();
    } else {
        tarotScene->displayCelticCross();
    }
}


void MainWindow::onGetReadingClicked() {
    // Check if we have an API key
    if (!mistralApi->hasApiKey()) {
        // Show dialog to get API key
        bool ok;
        QString apiKey = QInputDialog::getText(this, "Mistral API Key",
                                               "Please enter your Mistral AI API key:",
                                               QLineEdit::Normal, "", &ok);
        if (ok && !apiKey.isEmpty()) {
            mistralApi->setApiKey(apiKey);
        } else {
            dockControls->readingDisplay->setText("API key is required to generate readings.");
            return;
        }
    }
    if(tarotScene->getReadingRequested())
        return;

    // Get the prompt from TarotScene
    QString prompt = tarotScene->generateReadingPrompt();

    if (prompt.startsWith("Error:")) {
        dockControls->readingDisplay->setText(prompt);
        return;
    }

    // Show loading message
    dockControls->readingDisplay->setText("Generating reading...");

    // Generate the reading
    mistralApi->generateReading(prompt);
    tarotScene->setReadingRequested(true);
}

void MainWindow::onReadingReady(const QString& reading) {
    // Display the reading
    dockControls->readingDisplay->setText(reading);
}

void MainWindow::onApiError(const QString& errorMessage) {
    // Display the error
    dockControls->readingDisplay->setText("Error: " + errorMessage);
}

//save load

void MainWindow::onSaveReading() {
    if (tarotScene->getCurrentCards().isEmpty()) {
        QMessageBox::warning(this, "Save Error", "No cards to save.");
        return;
    }

    // Create TaroCaster directory in home directory if it doesn't exist
    QString saveDir = QDir::homePath() + "/TaroCaster";
    QDir dir;
    if (!dir.exists(saveDir)) {
        if (!dir.mkpath(saveDir)) {
            QMessageBox::warning(this, "Save Error",
                                 "Could not create save directory: " + saveDir);
            return;
        }
    }

    // Generate default filename with current date and time
    QString defaultFilename = "reading-" +
                              QDateTime::currentDateTime().toString("yyyy-MM-dd-HHmmss") +
                              ".tarot";

    QString fileName = QFileDialog::getSaveFileName(this, "Save Reading",
                                                    saveDir + "/" + defaultFilename,
                                                    "Tarot Readings (*.tarot)");
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Save Error",
                             "Could not open file for writing: " + file.errorString());
        return;
    }

    QJsonObject json;
    json["version"] = "1.0";
    json["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    json["spreadType"] = static_cast<int>(tarotScene->getCurrentSpreadType());
    json["reading"] = dockControls->readingDisplay->toPlainText();

    QJsonArray cardsArray;
    const QVector<CardLoader::CardData>& cards = tarotScene->getCurrentCards();
    for (int i = 0; i < cards.size(); i++) {
        QJsonObject cardObj;
        cardObj["number"] = cards[i].number;
        cardObj["reversed"] = cards[i].reversed;
        cardObj["position"] = i;
        cardsArray.append(cardObj);
    }
    json["cards"] = cardsArray;

    QJsonDocument doc(json);
    file.write(doc.toJson());
    file.close();

    QMessageBox::information(this, "Save Successful",
                             "Reading saved successfully to " + fileName);
}

void MainWindow::onLoadReading() {
    // Open file dialog in TaroCaster directory if it exists
    QString loadDir = QDir::homePath() + "/TaroCaster";
    QDir dir(loadDir);
    if (!dir.exists()) {
        loadDir = QDir::homePath();
    }

    QString fileName = QFileDialog::getOpenFileName(this, "Load Reading",
                                                    loadDir,
                                                    "Tarot Readings (*.tarot)");
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Load Error",
                             "Could not open file for reading: " + file.errorString());
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        QMessageBox::warning(this, "Load Error", "Invalid file format.");
        return;
    }

    QJsonObject json = doc.object();

    // Load cards
    QVector<CardLoader::CardData> loadedCards;
    if (json.contains("cards") && json["cards"].isArray()) {
        QJsonArray cardsArray = json["cards"].toArray();
        for (const QJsonValue& value : cardsArray) {
            if (value.isObject()) {
                QJsonObject cardObj = value.toObject();
                CardLoader::CardData card;
                card.number = cardObj["number"].toInt();
                card.reversed = cardObj["reversed"].toBool();
                loadedCards.append(card);
            }
        }
    }

    // Load spread type and display cards
    TarotScene::SpreadType spreadType = static_cast<TarotScene::SpreadType>(
        json["spreadType"].toInt(TarotScene::NoSpread));

    tarotScene->displaySavedSpread(spreadType, loadedCards);

    // Load reading
    if (json.contains("reading")) {
        dockControls->readingDisplay->setText(json["reading"].toString());
    }

    QMessageBox::information(this, "Load Successful",
                             "Reading loaded successfully from " + fileName);
}


void MainWindow::onEditApiKey()
{
    // Create a custom dialog with information
    QDialog dialog(this);
    dialog.setWindowTitle("Mistral API Key");

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    // Add information label with link
    QLabel *infoLabel = new QLabel("Enter your Mistral API key below. You can get a free API key from:");
    layout->addWidget(infoLabel);

    // Add clickable link
    QLabel *linkLabel = new QLabel("<a href='https://mistral.ai'>https://mistral.ai</a>");
    linkLabel->setOpenExternalLinks(true); // Makes the link clickable
    layout->addWidget(linkLabel);

    // Add line edit for API key
    QLineEdit *keyEdit = new QLineEdit();
    keyEdit->setText(mistralApi->getApiKey());
    keyEdit->setEchoMode(QLineEdit::Password); // Hide the API key
    layout->addWidget(keyEdit);

    // Add buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttonBox);

    // Show dialog and process result
    if (dialog.exec() == QDialog::Accepted) {
        QString newKey = keyEdit->text().trimmed();
        mistralApi->setApiKey(newKey);
    }
}
