#include "mainwindow.h"
#include<QMetaMethod>
#include <QtOpenGLWidgets/QtOpenGLWidgets>
#include <QSurfaceFormat>
#include <QStyleFactory>
#include"tarotorderdialog.h"
#include"Globals.h"
#include "donationdialog.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)

{
    this->setMinimumSize(1200, 800);
    this->showMaximized();

    setWindowTitle("TarotCaster");

    tarotScene = new TarotScene(this);

    tarotScene->setItemIndexMethod(QGraphicsScene::NoIndex); // Better for movable items
    tarotScene->setBackgroundBrush(Qt::black);

    centralView = new QGraphicsView(tarotScene);  // Set scene immediately

    centralView->setCacheMode(QGraphicsView::CacheBackground);
    centralView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    centralView->setBackgroundBrush(Qt::black);
    centralView->setRenderHint(QPainter::Antialiasing);
    centralView->setRenderHint(QPainter::SmoothPixmapTransform);
    centralView->setRenderHint(QPainter::TextAntialiasing, true);
    centralView->setOptimizationFlags(QGraphicsView::DontSavePainterState | QGraphicsView::DontAdjustForAntialiasing);
    centralView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);

#ifndef QT_NO_OPENGL
    QSurfaceFormat format;// = glWidget->format();
    format.setSamples(4);  // 4x MSAA
    format.setSwapInterval(1);  // Enable VSync
    format.setColorSpace(QSurfaceFormat::sRGBColorSpace); // Better color accuracy
    //use either this or     glWidget->setFormat(format);
    //QSurfaceFormat::setDefaultFormat(format);  // Must be set before glWidget is created
    QOpenGLWidget *glWidget = new QOpenGLWidget();
    glWidget->setFormat(format);
    centralView->setViewport(glWidget);
#endif
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
    //view menu
    // Create View menu
    QMenu* viewMenu = menuBar()->addMenu("&View");

    // Add toggle actions for dock widgets
    QAction* toggleLeftDockAction = viewMenu->addAction("&Left Panel");
    toggleLeftDockAction->setCheckable(true);
    toggleLeftDockAction->setChecked(true); // Visible by default
    toggleLeftDockAction->setShortcut(QKeySequence("Ctrl+L")); // Optional shortcut
    connect(toggleLeftDockAction, &QAction::toggled, leftDock, &QDockWidget::setVisible);

    QAction* toggleRightDockAction = viewMenu->addAction("&Right Panel");
    toggleRightDockAction->setCheckable(true);
    toggleRightDockAction->setChecked(true); // Visible by default
    toggleRightDockAction->setShortcut(QKeySequence("Ctrl+R")); // Optional shortcut
    connect(toggleRightDockAction, &QAction::toggled, rightDock, &QDockWidget::setVisible);

    //
    // Edit Menu
    QMenu *editMenu = menuBar()->addMenu("&Edit");
    editMenu->addAction("&Mistral API Key", this, &MainWindow::onEditApiKey);

    // Settings Menu
    QMenu *settingsMenu = menuBar()->addMenu("&Settings");
    // We'll populate this later
    settingsMenu->addAction("&Preferences", this, &MainWindow::onOpenPreferences);

    // Tools Menu (create if it doesn't exist)
    QMenu* toolsMenu = menuBar()->addMenu("&Tools");

    // Add custom spread designer action
    QAction* createSpreadAction = toolsMenu->addAction("&Custom Spread Designer");
    connect(createSpreadAction, &QAction::triggered, this, &MainWindow::onCreateCustomSpreadClicked);

    // ImportDeck
    QAction* importDeckAction = toolsMenu->addAction("&Import Deck");
    connect(importDeckAction, &QAction::triggered, this, &MainWindow::orderDeck);
    // Help Menu
    QMenu *helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("&About", this, &MainWindow::onShowAbout);
    helpMenu->addAction("&Instructions", this, &MainWindow::onShowInstructions);
    helpMenu->addAction("&Spreads", this, &MainWindow::onShowSpreads);
    helpMenu->addAction("&How to Add Additional Decks", this, &MainWindow::onShowAddDecks);
    helpMenu->addAction("&How to Add Custom Spreads", this, &MainWindow::onShowAddCustomSpreads);
    helpMenu->addAction("&Custom Spreads", this, &MainWindow::onShowCustomSpreads);
    helpMenu->addAction("&ChangeLog", this, &MainWindow::onShowChangelog);
    QAction *supportAction = helpMenu->addAction(tr("Support Us"));
        connect(supportAction, &QAction::triggered, this, [this]() {
            DonationDialog supportusDialog(this);
            supportusDialog.setObjectName("donationDialog");
            supportusDialog.setStyleSheet("background-color: white; color: black;");
            supportusDialog.exec();
        });
    connect(dockControls->getZoomSlider(), &QSlider::valueChanged, this, &MainWindow::handleZoomSlider);

    connect(tarotScene, &TarotScene::viewRefreshRequested,[this]{
        //clearQuestionButton->click();
        meaningDisplay->clear();
    });
}

MainWindow::~MainWindow()
{
    // The OpenGL widget might not have a proper parent
    QOpenGLWidget* glWidget = qobject_cast<QOpenGLWidget*>(centralView->viewport());
    if (glWidget && glWidget->parent() == nullptr) {
        delete glWidget;
    }
}



void MainWindow::createDocks() {
    //leftDock = new QDockWidget("Deck Controls and Readings",this);
    leftDock = new QDockWidget("Decks and Spreads", this);
    //leftDock = new QDockWidget("Decks and Readings", this);

    //leftDock->setStyleSheet("QDockWidget::title { text-align: center; }");

    //leftDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    leftDock->setFeatures(QDockWidget::DockWidgetClosable); // Makes it hideable

    dockControls = new DockControls(this);
    leftDock->setWidget(dockControls);
    leftDock->setAllowedAreas(Qt::LeftDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, leftDock);


    //
    rightDock = new QDockWidget("Card Meanings", this);
    //rightDock->setStyleSheet("QDockWidget::title { text-align: center; }");
    leftDock->setFeatures(QDockWidget::DockWidgetClosable);

    // Create a container widget for the question input and meaning display
    QWidget *rightDockWidget = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightDockWidget);

    // Create question section
    /*
    QGroupBox *questionGroup = new QGroupBox("Question", this);
    QVBoxLayout *questionLayout = new QVBoxLayout(questionGroup);

    // Create question input and button
    QTextEdit *questionInput = new QTextEdit(this);
    questionInput->setPlaceholderText("Enter your question for the reading...");
    questionInput->setMaximumHeight(80);

    QPushButton *setQuestionButton = new QPushButton("Set Question", this);

    questionLayout->addWidget(questionInput);
    questionLayout->addWidget(setQuestionButton);

    // Add question section to right layout
    rightLayout->addWidget(questionGroup);
    */


    QGroupBox *questionGroup = new QGroupBox("Question", this);
    QVBoxLayout *questionLayout = new QVBoxLayout(questionGroup);

    // Create question input and button
    questionInput = new QTextEdit(this);
    questionInput->setPlaceholderText("Enter your question for the reading...");
    questionInput->setMaximumHeight(80);

    // Create buttons and arrange them horizontally
    QPushButton *setQuestionButton = new QPushButton("Set Question", this);
    clearQuestionButton = new QPushButton("Clear", this);

    // Create horizontal layout for buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(setQuestionButton);
    buttonLayout->addWidget(clearQuestionButton);

    questionLayout->addWidget(questionInput);
    questionLayout->addLayout(buttonLayout);  // Add the horizontal button layout

    // Add question section to right layout
    rightLayout->addWidget(questionGroup);

    // Add meaning display
    meaningDisplay = new MeaningDisplay(this);
    rightLayout->addWidget(meaningDisplay, 1); // Give meaning display most space

    rightDock->setWidget(rightDockWidget);
    rightDock->setAllowedAreas(Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, rightDock);
    //
    // Connect the new signals
    connect(dockControls, &DockControls::deckLoaded,
            this, &MainWindow::onDeckLoaded);
    connect(dockControls, &DockControls::reversedCardsToggled,
            this, &MainWindow::onReversedCardsToggled);
    connect(dockControls, &DockControls::swapEightElevenToggled,
            this, &MainWindow::onSwapEightElevenToggled);
    connect(setQuestionButton, &QPushButton::clicked, [this]{
        QString question = questionInput->toPlainText().trimmed();
        if (question.isEmpty()) return;
        mistralApi->setQuery(question);
        QMessageBox::information(this, "Question Set", "Your question has been set.\n"
                                                       "Now deal the cards and request ai for interpretation.");
    });

    connect(clearQuestionButton, &QPushButton::clicked, [this]{
        questionInput->clear();
        mistralApi->setQuery(QString());
    });
}



void MainWindow::clearMeaningDisplay()
{
    meaningDisplay->clear();
    dockControls->readingDisplay->clear();

}

void MainWindow::showCardMeaning(int cardNumber) {
    // Apply Justice/Strength numbering swap if enabled
    if (swapEightEleven) {
        if (cardNumber == 8) {
            cardNumber = 11;  // Treat 8 as Strength (XI)
        } else if (cardNumber == 11) {
            cardNumber = 8;   // Treat 11 as Justice (VIII)
        }
    }
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

void MainWindow::onSwapEightElevenToggled(bool allowed)
{
    if (allowed){
        swapEightEleven = true;
        tarotScene->swapEightEleven = true;

    }else{
        swapEightEleven = false;
        tarotScene->swapEightEleven = false;
    }
}


void MainWindow::onDealClicked() {
    readingDisplayed = false;
    QString currentSpread = dockControls->spreadSelector->currentText();

    // Check for built-in spreads first
    if (currentSpread == "Single Card") {
        tarotScene->displaySingleCard();
    } else if (currentSpread == "Three Card") {
        tarotScene->displayThreeCardSpread();
    } else if (currentSpread == "Horseshoe") {
        tarotScene->displayHorseshoeSpread();
    } else if (currentSpread == "ZodiacSpread") {
        tarotScene->displayZodiacSpread();
    } else if (currentSpread == "Celtic Cross") {
        tarotScene->displayCelticCross();
    } else {
        // Check if it's a custom spread by trying to display it through our method
        if (!onDisplayCustomSpread(currentSpread)) {
            // If display fails, fall back to Celtic Cross
            tarotScene->displayCelticCross();
        }
    }
}





void MainWindow::onGetReadingClicked() {
    if(tarotScene->getReadingRequested() && readingDisplayed) return;


    dockControls->readingDisplay->clear();
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


    // Get the prompt from TarotScene
    QString prompt = tarotScene->generateReadingPrompt();

    if (prompt.startsWith("Error:")) {
        dockControls->readingDisplay->setText(prompt);
        readingDisplayed = false;
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
    QString html = markdownToHtml(reading);
    dockControls->readingDisplay->setText(html);
    readingDisplayed = true;
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

    //
    // Add this section to save the custom spread name
    if (tarotScene->getCurrentSpreadType() == TarotScene::Custom) {
        json["customSpreadName"] = currentCustomSpreadName; // You need to track this
    }
    //

    json["reading"] = dockControls->readingDisplay->toPlainText();
    json["query"] = questionInput->toPlainText().trimmed();

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

    //
    // Add this section to handle custom spreads
    if (spreadType == TarotScene::Custom) {
        QString customSpreadName = json["customSpreadName"].toString();
        tarotScene->displaySavedSpread(spreadType, loadedCards, customSpreadName);
    } else {
        tarotScene->displaySavedSpread(spreadType, loadedCards);
    }
    //

    //tarotScene->displaySavedSpread(spreadType, loadedCards);

    // Load reading
    if (json.contains("reading")) {
        dockControls->readingDisplay->setText(json["reading"].toString());
    }

    if (json.contains("query")) {
        questionInput->setText(json["query"].toString());
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



void MainWindow::onCreateCustomSpreadClicked()
{
    // Check if tarotScene is valid
    if (!tarotScene) {
        QMessageBox::critical(this, "Error", "Tarot scene is not initialized");
        return;
    }
    tarotScene->clearScene();
    // Create the custom spread designer dialog
    CustomSpreadDesigner* designer = new CustomSpreadDesigner(tarotScene, this);

    // Set delete-on-close attribute to ensure proper cleanup
    designer->setAttribute(Qt::WA_DeleteOnClose);
    designer->setAttribute(Qt::WA_TranslucentBackground);
    designer->setWindowOpacity(0.8);
    //
    //QRect mainGeometry = this->geometry();
    //QPoint dialogPos = mainGeometry.topRight() + QPoint(20, 50); // 20px right, 50px down
    //designer->move(dialogPos);
    //

    // Show the dialog
    designer->show();

}

bool MainWindow::onDisplayCustomSpread(const QString& spreadName) {
    bool success = tarotScene->displayCustomSpread(spreadName);
    if (success) {
        currentCustomSpreadName = spreadName;
    }
    return success;
}

void MainWindow::onShowCustomSpreads()
{
    HelpDialog *dialog = new HelpDialog(HelpDialog::CustomSpreads, this);
    dialog->setTarotScene(tarotScene);

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setModal(false);  // Make the dialog non-modal
    dialog->show();  // Use show() instead of exec() for non-modal behavior
}

void MainWindow::onShowAddCustomSpreads()
{
    HelpDialog *dialog = new HelpDialog(HelpDialog::CustomSpreadHelp, this);

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setModal(false);  // Make the dialog non-modal
    dialog->show();  // Use show() instead of exec() for non-modal behavior
}

void MainWindow::onShowChangelog()
{
    HelpDialog *dialog = new HelpDialog(HelpDialog::ChangeLogHelp, this);

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setModal(false);  // Make the dialog non-modal
    dialog->show();  // Use show() instead of exec() for non-modal behavior
}

void MainWindow::orderDeck()
{
    QMessageBox::StandardButton reply = QMessageBox::information(
        this,
        "Deck Preparation Assistant",
        QString("This will help you assign names to your Tarot card images that are compatible with TarotCaster expected format.\n\n"
                "Please place a COPY of your unordered deck in:\n%1\n\n"
                "Supported formats: PNG, JPG\n\n"
                "Click OK to proceed or Cancel to exit and copy your deck first.")
            .arg(getUserDecksDirPath() + "/"),
        QMessageBox::Ok | QMessageBox::Cancel,
        QMessageBox::Ok
        );

    if (reply != QMessageBox::Ok) {
        return;  // User canceled
    }

    QString deckPath = QFileDialog::getExistingDirectory(this, "Select Deck Directory",
                                                         getUserDecksDirPath());
    if (deckPath.isEmpty()) return;

    TarotOrderDialog dialog(deckPath, this);
    dialog.exec();

}

void MainWindow::handleZoomSlider(int value)
{
    qreal newZoom = value / 100.0; // Convert percentage to factor

    // Calculate the transformation needed
    QTransform transform;
    transform.scale(newZoom, newZoom);
    centralView->setTransform(transform);

    currentZoomLevel = newZoom;
}


QString MainWindow::markdownToHtml(const QString &markdown)
{
    QString html = markdown;

    // Convert headers
    html.replace(QRegularExpression("^###### (.*)$", QRegularExpression::MultilineOption), "<h6>\\1</h6>");
    html.replace(QRegularExpression("^##### (.*)$", QRegularExpression::MultilineOption), "<h5>\\1</h5>");
    html.replace(QRegularExpression("^#### (.*)$", QRegularExpression::MultilineOption), "<h4>\\1</h4>"); 
    html.replace(QRegularExpression("^### (.*)$", QRegularExpression::MultilineOption), "<h3>\\1</h3>");
    html.replace(QRegularExpression("^## (.*)$", QRegularExpression::MultilineOption), "<h2>\\1</h2>");
    html.replace(QRegularExpression("^# (.*)$", QRegularExpression::MultilineOption), "<h1>\\1</h1>");

    // Convert bold (**text**)
    html.replace(QRegularExpression("\\*\\*(.*?)\\*\\*"), "<b>\\1</b>");

    // Convert italic (*text*)
    html.replace(QRegularExpression("\\*(.*?)\\*"), "<i>\\1</i>");

    // Convert bullet points
    //html.replace(QRegularExpression("^\\- (.*)$", QRegularExpression::MultilineOption), "• \\1<br>");
    //html.replace(QRegularExpression("^\\- (.*)$", QRegularExpression::MultilineOption), "• \\1<br>");

    // Handle • bullets (with optional whitespace)
    html.replace(QRegularExpression("^[\\s]*\\•[\\s]+(.*)$", QRegularExpression::MultilineOption), "• \\1<br>");

    // Handle - bullets (with optional whitespace)
    html.replace(QRegularExpression("^[\\s]*\\-[\\s]+(.*)$", QRegularExpression::MultilineOption), "• \\1<br>");

    // Handle * bullets (with optional whitespace)
    html.replace(QRegularExpression("^[\\s]*\\*[\\s]+(.*)$", QRegularExpression::MultilineOption), "• \\1<br>");

    // Convert horizontal rules (---, ***, ___) with optional spaces
    html.replace(QRegularExpression("^\\s*(---|\\*\\*\\*|___)\\s*$", QRegularExpression::MultilineOption), "<hr>");

    html.replace("\n", "<br>");

    //return "<html><body>" + html + "</body></html>";
    return html;
}
