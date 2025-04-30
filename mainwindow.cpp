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

   // QString jsonPath = QDir(QCoreApplication::applicationDirPath()).filePath("card_meanings.json");
    QString jsonPath = ":/resources/card_meanings.json";


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

    // Edit Menu
    QMenu *editMenu = menuBar()->addMenu("&Edit");
    editMenu->addAction("&Mistral API Key", this, &MainWindow::onEditApiKey);

    // Settings Menu
    QMenu *settingsMenu = menuBar()->addMenu("&Settings");
    // We'll populate this later
    settingsMenu->addAction("&Preferences", this, &MainWindow::onOpenPreferences);

    // Help Menu
    QMenu *helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("&About", this, &MainWindow::onShowAbout);
    helpMenu->addAction("&Instructions", this, &MainWindow::onShowInstructions);
    helpMenu->addAction("&Spreads", this, &MainWindow::onShowSpreads);
    helpMenu->addAction("&How to Add Additional Decks", this, &MainWindow::onShowAddDecks);


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
    //leftDock = new QDockWidget("Deck Controls and Readings",this);
    leftDock = new QDockWidget("Decks and Spreads", this);
    //leftDock = new QDockWidget("Decks and Readings", this);

    //leftDock->setStyleSheet("QDockWidget::title { text-align: center; }");

    leftDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    dockControls = new DockControls(this);
    leftDock->setWidget(dockControls);
    leftDock->setAllowedAreas(Qt::LeftDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, leftDock);

    rightDock = new QDockWidget("Card Meaning",this);
    //rightDock->setStyleSheet("QDockWidget::title { text-align: center; }");

    rightDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
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
    } else if (currentSpread == "ZodiacSpread") {
        tarotScene->displayZodiacSpread();
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


void MainWindow::onSaveReading() {
    if (tarotScene->getCurrentCards().isEmpty()) {
        QMessageBox::warning(this, "Save Error", "No cards to save.");
        return;
    }

    // Get the proper data location for the application
    QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dataLocation.isEmpty()) {
        dataLocation = QDir::homePath() + "/.local/share/TaroCaster";
    }

    // Create the directory if it doesn't exist
    QDir dir;
    if (!dir.exists(dataLocation)) {
        if (!dir.mkpath(dataLocation)) {
            QMessageBox::warning(this, "Save Error",
                                 "Could not create save directory: " + dataLocation);
            return;
        }
    }

    // Generate default filename with current date and time
    QString defaultFilename = "reading-" +
                              QDateTime::currentDateTime().toString("yyyy-MM-dd-HHmmss") +
                              ".tarot";

    QString fileName = QFileDialog::getSaveFileName(this, "Save Reading",
                                                    dataLocation + "/" + defaultFilename,
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
    // Get the proper data location for the application
    QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dataLocation.isEmpty()) {
        dataLocation = QDir::homePath() + "/.local/share/TaroCaster";
    }

    // Use the data location if it exists, otherwise fall back to home directory
    QDir dir(dataLocation);
    QString loadDir = dir.exists() ? dataLocation : QDir::homePath();

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
    //QLabel *linkLabel = new QLabel("<a href='https://mistral.ai'>https://mistral.ai</a>");
    QLabel *linkLabel = new QLabel("<a href='https://mistral.ai' style='color: gold;'>https://mistral.ai</a>");

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


void MainWindow::onOpenPreferences()
{
    // We'll implement this later
    QMessageBox::information(this, "Settings", "Preferences dialog will be implemented soon.");
}

void MainWindow::onShowAbout()
{
    HelpDialog *dialog = new HelpDialog(HelpDialog::About, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

void MainWindow::onShowInstructions()
{
    HelpDialog *dialog = new HelpDialog(HelpDialog::Instructions, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setModal(false);  // Make the dialog non-modal
    dialog->show();  // Use show() instead of exec() for non-modal behavior
}

void MainWindow::onShowSpreads()
{
    HelpDialog *dialog = new HelpDialog(HelpDialog::Spreads, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setModal(false);  // Make the dialog non-modal
    dialog->show();  // Use show() instead of exec() for non-modal behavior
}

void MainWindow::onShowAddDecks() {
    HelpDialog *dialog = new HelpDialog(HelpDialog::AddDecks, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setModal(false);  // Make the dialog non-modal
    dialog->show();  // Use show() instead of exec() for non-modal behavior
}
