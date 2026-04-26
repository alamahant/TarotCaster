#include "mainwindow.h"
#include<QMetaMethod>
#include <QtOpenGLWidgets/QtOpenGLWidgets>
#include <QSurfaceFormat>
#include <QStyleFactory>
#include"tarotorderdialog.h"
#include"Globals.h"
#include "donationdialog.h"
#include "modelselectordialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , questionInput(new QTextEdit(this))

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

    QAction *openFolderAction = fileMenu->addAction("&Open Data Directory");
    connect(openFolderAction, &QAction::triggered, this, &MainWindow::openFolder);
    fileMenu->addSeparator();

    fileMenu->addSeparator();
    QAction *createSymlinkAction = fileMenu->addAction("Create Shortcut to TarotCaster Data");
    connect(createSymlinkAction, &QAction::triggered, this, &MainWindow::createSymlink);




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



    // Settings Menu
    QMenu *settingsMenu = menuBar()->addMenu("&Settings");

    //
    QAction *aiModelsAction = settingsMenu->addAction("Configure AI &Models...", this, &MainWindow::configureAIModels);



    QAction *checkModelAction = settingsMenu->addAction("Check AI Model &Status", this, [this]() {
        if (!activeModelLoaded) {
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("AI Model Not Configured");
            msgBox.setText("No active AI model found. You need to configure a model to get chart interpretations.");
            msgBox.setInformativeText("Would you like to configure one now?\n\n"
                                      "Note: If you've been using Mistral, you can add it as a provider with your API key.");

            QPushButton *configureButton = msgBox.addButton("Configure Models", QMessageBox::ActionRole);
            QPushButton *closeButton = msgBox.addButton(QMessageBox::Close);

            msgBox.exec();

            if (msgBox.clickedButton() == configureButton) {
                configureAIModels();
            }
        } else {
            // Load the active model settings
            QSettings settings;
            settings.beginGroup("Models");
            QString activeModelName = settings.value("ActiveModel").toString();
            settings.beginGroup(activeModelName);

            QString provider = settings.value("provider").toString();
            QString endpoint = settings.value("endpoint").toString();
            QString modelName = settings.value("modelName").toString();
            QString apiKey = settings.value("apiKey").toString();
            double temperature = settings.value("temperature", 0.7).toDouble();
            int maxTokens = settings.value("maxTokens", 8192).toInt();

            settings.endGroup();
            settings.endGroup();

            // Build status message
            QString statusMessage = QString(
                "<b>Active Model:</b> %1<br><br>"
                "<b>Provider:</b> %2<br>"
                "<b>Model:</b> %3<br>"
                "<b>Endpoint:</b> %4<br>"
                "<b>Temperature:</b> %5<br>"
                "<b>Max Tokens:</b> %6<br>"
                "<b>API Key:</b> %7"
            ).arg(activeModelName)
             .arg(provider)
             .arg(modelName)
             .arg(endpoint)
             .arg(temperature)
             .arg(maxTokens)
             .arg(apiKey.isEmpty() ? "<font color='red'><b>MISSING</b></font>" : "<font color='green'><b>Configured</b></font>");

            // Check if API key is missing for cloud providers (not local)
            if (apiKey.isEmpty() && !endpoint.contains("localhost") && !endpoint.contains("127.0.0.1")) {
                statusMessage += "<br><br><font color='red'><b>WARNING:</b> This appears to be a cloud provider but no API key is set. Interpretations will fail.</font>";
            }

            QMessageBox::information(this, "AI Model Status", statusMessage);
        }
    });
    checkModelAction->setIcon(QIcon::fromTheme("dialog-information"));
    //

    //settingsMenu->addAction("&Preferences", this, &MainWindow::onOpenPreferences);

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

    // Create question button
    openQuestionDialogButton = new QPushButton("Set a Question", this);
    openQuestionDialogButton->setToolTip("Pose a question to be forwarded to AI");
    connect(openQuestionDialogButton, &QPushButton::clicked, this, &MainWindow::onSetQuestion);
    rightLayout->addWidget(openQuestionDialogButton);
    //

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
    if (!activeModelLoaded) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("AI Model Not Configured");
        msgBox.setText("No active AI model found.");
        msgBox.setInformativeText("Please configure an AI model in Settings → Configure AI Models before generating readings.\n\n"
                                  "You can use Mistral (get your free key at mistral.ai), OpenAI, Groq, or local models like Ollama.");

        QPushButton *openSettingsButton = msgBox.addButton("Open Settings", QMessageBox::ActionRole);
        QPushButton *closeButton = msgBox.addButton(QMessageBox::Close);

        msgBox.exec();

        if (msgBox.clickedButton() == openSettingsButton) {
            // Open your model selector dialog
            ModelSelectorDialog dlg(this);
            dlg.exec();

            // Reload model after dialog closes
            activeModelLoaded = mistralApi->loadActiveModel();

            // If still not loaded, show error
            if (!activeModelLoaded) {
                dockControls->readingDisplay->setText("No AI model configured. Please configure one and try again.");
            }
        } else {
            dockControls->readingDisplay->setText("AI model configuration is required to generate readings.");
        }
        return;
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

    if (!questionInput) {
        QMessageBox::warning(this, "Save Error", "Please set a question first!");
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



void MainWindow::configureAIModels()
{
    ModelSelectorDialog dlg(this);

    // Connect to activeModelChanged signal
       connect(&dlg, &ModelSelectorDialog::activeModelChanged, this, [this](const QString &modelName) {
           // When active model changes, reload it in MistralAPI
           // This will update GlobalFlags::activeModelLoaded internally
           mistralApi->loadActiveModel();

           // Optional: Show status message
           statusBar()->showMessage(tr("Active model changed to: %1").arg(modelName), 3000);
       });

       dlg.exec();  // Just show the dialog, no need to process results

        mistralApi->loadActiveModel();

       // The dialog saves changes to QSettings automatically
       // The MistralAPI class will read the active model from QSettings when needed
}


void MainWindow::onSetQuestion() {
    // Create dialog and widgets only once
    if (!questionDialog) {
        questionDialog = new QDialog(this);
        questionDialog->setWindowTitle("Set Your Question");
        questionDialog->resize(500, 300);

        QVBoxLayout *mainLayout = new QVBoxLayout(questionDialog);

        // Question group
        QGroupBox *questionGroup = new QGroupBox("Your Question", questionDialog);
        QVBoxLayout *questionLayout = new QVBoxLayout(questionGroup);

        // Create question input
        //questionInput = new QTextEdit(questionDialog);
        questionInput->setToolTip("Your question will be forwarded to the AI for interpretation");
        questionInput->setPlaceholderText("Enter your question for the reading...");
        questionInput->setMaximumHeight(100);

        // Create buttons
        QPushButton* setQuestionButton = new QPushButton("Set Question", questionDialog);
        QPushButton* clearQuestionButton = new QPushButton("Clear", questionDialog);

        // Connect buttons
        connect(setQuestionButton, &QPushButton::clicked, this, [this]() {

            QString question = questionInput->toPlainText().trimmed();
            if (question.isEmpty()) return;
            currentQuestion = question;
            mistralApi->setQuery(question);
            openQuestionDialogButton->setToolTip("Pose a question to be forwarded to AI\n\n"
                                          "Current question:\n\n"
                                          + question);




            QMessageBox::information(this, "Question Set", "Your question has been set.\n"
                                                           "Now deal the cards and request ai for interpretation.");
        });
        connect(clearQuestionButton, &QPushButton::clicked, this, [this]() {
            questionInput->clear();
            currentQuestion.clear();
        });

        QPushButton* closeQuestionDialogButton = new QPushButton("Close", questionDialog);
        connect(closeQuestionDialogButton, &QPushButton::clicked, this, [this](){
            questionDialog->hide();
        });

        // Create horizontal layout for buttons
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        buttonLayout->addWidget(setQuestionButton);
        buttonLayout->addWidget(clearQuestionButton);

        questionLayout->addWidget(questionInput);
        questionLayout->addLayout(buttonLayout);
        questionLayout->addWidget(closeQuestionDialogButton);

        mainLayout->addWidget(questionGroup);

    }

    // Update with existing question if any
    if (!currentQuestion.isEmpty()) {
        questionInput->setText(currentQuestion);
    } else {
        questionInput->clear();
    }

    questionDialog->show();
}

////////////

void MainWindow::createSymlink()
{
#ifdef FLATPAK_BUILD
    QString msg = "";
    QMessageBox msgBox;
    msgBox.setWindowTitle("Flatpak Permission Required");
    msgBox.setText(QString(
                       "%1 is running as a Flatpak and may not have access to your home directory.\n\n"
                       "To create a symlink, you may need to grant home directory access first.\n\n"
                       "Option 1 - Terminal:\n"
                       "  Grant access:\n"
                       "    flatpak override --user --filesystem=home io.github.alamahant.%1\n\n"
                       "  Revoke access later:\n"
                       "    flatpak override --user --nofilesystem=home io.github.alamahant.%1\n\n"
                       "Option 2 - Flatseal:\n"
                       "  Install Flatseal from Flathub and grant 'Home' access to %1.\n\n"
                       "If you have already granted permissions, you can continue."
                       ).arg(QApplication::applicationName()));

    msgBox.setIcon(QMessageBox::Information);

    QPushButton *continueButton = msgBox.addButton("Continue", QMessageBox::AcceptRole);
    QPushButton *cancelButton = msgBox.addButton("Cancel", QMessageBox::RejectRole);
    msgBox.setDefaultButton(cancelButton);

    msgBox.exec();

    if (msgBox.clickedButton() != continueButton) {
        return; // User cancelled
    }

#endif
    // Open dialog to select destination folder
    QString destinationDir = QFileDialog::getExistingDirectory(
                this,
                "Select Destination Folder for Symlink",
                QDir::homePath(),
                QFileDialog::ShowDirsOnly
                );

    if (destinationDir.isEmpty()) {
        return; // User cancelled
    }

    // Create symlink path
    QString symlinkPath = QDir(destinationDir).filePath(QApplication::applicationName());
    // Check if symlink already exists
    if (QFile::exists(symlinkPath) || QFileInfo(symlinkPath).isSymLink()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
                    this,
                    "Symlink Exists",
                    QString("A file or symlink already exists at:\n%1\n\nOverwrite?").arg(symlinkPath),
                    QMessageBox::Yes | QMessageBox::No
                    );

        if (reply != QMessageBox::Yes) {
            return;
        }

        // Remove existing file/symlink
        if (!QFile::remove(symlinkPath)) {
            QMessageBox::warning(this, "Error", "Could not remove existing file/symlink");
            return;
        }
    }

    // Create the symlink
    QString targetPath = getLocalDataDirPath();

    if (!QFile::exists(targetPath)) {
        QMessageBox::warning(this, "Error",
                             QString("Target directory does not exist:\n%1").arg(targetPath));
        return;
    }

    if (QFile::link(targetPath, symlinkPath)) {
        QMessageBox::information(
                    this,
                    "Symlink Created",
                    QString("Symlink created successfully!\n\n"
                            "Name: %3\n"
                            "Location: %1\n\n"
                            "Now you can access %3 data from:\n%2")
                    .arg(destinationDir)
                    .arg(symlinkPath)
                    .arg(QApplication::applicationName())
                    );
    } else {
        QMessageBox::warning(
                    this,
                    "Error",
                    QString("Failed to create symlink.\n\n"
                            "Destination: %1\n"
                            "Target: %2\n\n"
                            "Possible reasons:\n"
                            "• Insufficient permissions\n"
                            "• Invalid destination path\n"
                            "• Filesystem doesn't support symlinks")
                    .arg(symlinkPath)
                    .arg(targetPath)
                    );
    }
}

void MainWindow::openFolder() {
    // Optional: Check if the folder exists
    QDir dir(getLocalDataDirPath());
    if (!dir.exists()) {
        return;
    }

    // Convert local path to URL and open
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(getLocalDataDirPath()))) {
    }
}
