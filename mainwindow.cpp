#include "mainwindow.h"
#include<QMetaMethod>
#include <QtOpenGLWidgets/QtOpenGLWidgets>
#include <QSurfaceFormat>
#include <QStyleFactory>
#include"tarotorderdialog.h"
#include"Globals.h"
#include "donationdialog.h"
#include "modelselectordialog.h"
#include"journalmanager.h"
#include"socialsharedialog.h"
#include<QCoreApplication>
#include<QMimeData>
#include"helpdialog.h"
#include"importphysicaldialog.h"
#include<QProcess>
#include<QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , questionInput(new QTextEdit(this))
    , journalDialog(new JournalDialog(this))
    , m_socialShare(new SocialShare(this))
    , rssDialog(new RssNotificationDialog(this))
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
    centralView->setRenderHint(QPainter::Antialiasing, true);
    centralView->setRenderHint(QPainter::SmoothPixmapTransform, true);
    centralView->setRenderHint(QPainter::TextAntialiasing, true);
    //centralView->setOptimizationFlags(QGraphicsView::DontSavePainterState | QGraphicsView::DontAdjustForAntialiasing);
    centralView->setOptimizationFlags(QGraphicsView::DontSavePainterState);
    //centralView->setRenderHint(QPainter::LosslessImageRendering, true);


#ifndef QT_NO_OPENGL
    QSurfaceFormat format;// = glWidget->format();
    format.setSamples(4);  // 4x MSAA was 4
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

    connect(clearButton, &QPushButton::clicked,
            tarotScene, &TarotScene::clearScene);

    connect(clearButton, &QPushButton::clicked,
            this, &MainWindow::clearMeaningDisplay);

   connect(dockControls->allowReversed, &QCheckBox::toggled,
           tarotScene, &TarotScene::setAllowReversedCards);

    connect(dockControls, &DockControls::displayFullDeckRequested,
            tarotScene, &TarotScene::displayFullDeck);

   connect(displayFullDeckButton, &QPushButton::clicked,
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
    ;

    fileMenu->addSeparator();
#ifndef Q_OS_WIN
    QAction *createSymlinkAction = fileMenu->addAction("Create Shortcut to TarotCaster Data");
    connect(createSymlinkAction, &QAction::triggered, this, &MainWindow::createSymlink);
    fileMenu->addSeparator();
#endif

    rssAction = fileMenu->addAction("&RSS Notifications");
    rssAction->setShortcut(QKeySequence("Ctrl+R"));
    rssAction->setIcon(QIcon(":/resources/icons-white/rss.svg"));
    connect(rssAction, &QAction::triggered, this, [this]{
        if(rssDialog) rssDialog->show();
    });
    fileMenu->addAction(rssAction);
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
    settingsMenu->addSeparator();

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
    settingsMenu->addSeparator();

    QAction* aiModelInfoAction = new QAction("AI Model Info Guide", this);
    connect(aiModelInfoAction, &QAction::triggered, this, &MainWindow::showAIConfigGuide);
    settingsMenu->addAction(aiModelInfoAction);
    //

    settingsMenu->addSeparator();
    //
    QAction* scaleAction = settingsMenu->addAction("Set &Scale...");
    scaleAction->setShortcut(QKeySequence("Ctrl+Shift+S"));
    connect(scaleAction, &QAction::triggered, this, [this]() {
        QSettings settings;
        bool ok = false;
        double current = settings.value("ui/scaleFactor", 1.0).toDouble();

        double factor = QInputDialog::getDouble(
            this, tr("UI Scale"),
            tr("Scale factor (e.g. 0.9, 1.0, 1.1, 1.25):"),
            current,          // initial value
            0.5,              // min
            3.0,              // max
            2,                // decimals shown
            &ok,
            Qt::WindowFlags(),
            0.05);             // ← step

        if (!ok) return;

        settings.setValue("ui/scaleFactor", factor);
        settings.sync();

        QMessageBox msg(this);
        msg.setWindowTitle(tr("Restart Required"));
        msg.setIcon(QMessageBox::Information);
        msg.setText(tr("Please restart the application to apply the new scale."));
        msg.setInformativeText(tr(
            "If the new scale makes the app unusable, delete the settings file:\n\n%1\n\n"
            "Note: this will reset all user-defined settings.")
            .arg(QSettings().fileName()));


        QPushButton *copyBtn   = msg.addButton(tr("Copy Command"), QMessageBox::ActionRole);
        QPushButton *cancelBtn = msg.addButton(tr("Cancel"), QMessageBox::RejectRole);
        QPushButton *okBtn     = msg.addButton(tr("OK"), QMessageBox::AcceptRole);

        for (;;) {
            msg.exec();
            if (msg.clickedButton() != copyBtn)
                break;

        #ifdef Q_OS_WIN
            const QString cmd = QString("Remove-Item \"%1\"").arg(QSettings().fileName());
        #else
            const QString cmd = QString("rm \"%1\"").arg(QSettings().fileName());
        #endif
            QGuiApplication::clipboard()->setText(cmd);

            msg.setInformativeText(tr(
                "Command copied to clipboard:\n\n%1\n\n"
                "If the new scale makes the app unusable, delete the settings file above.")
                .arg(cmd));
        }

        if (msg.clickedButton() == okBtn)
            qApp->quit();
        // Cancel: nothing happens

    });

    settingsMenu->addSeparator();
    QAction* fontAction = settingsMenu->addAction("Set &Font Size...");
    fontAction->setShortcut(QKeySequence("Ctrl+Shift+F"));
    connect(fontAction, &QAction::triggered, this, [this]() {
        QSettings settings;

        QMessageBox box(this);
        box.setWindowTitle(tr("Font Size"));
        box.setIcon(QMessageBox::NoIcon);
        box.setText(tr("Point size (e.g. 9, 10, 12, 14):"));

        QSpinBox *spin = new QSpinBox(&box);
        spin->setRange(6, 32);
        spin->setSingleStep(1);

        double current = settings.value("ui/fontSize", 0.0).toDouble();
        if (current <= 0.0)
            current = qApp->font().pointSizeF();
        spin->setValue(qRound(current));

        if (auto *grid = qobject_cast<QGridLayout*>(box.layout())) grid->addWidget(spin, 1, 1);
        QPushButton *okBtn     = box.addButton(QMessageBox::Ok);
        QPushButton *cancelBtn = box.addButton(QMessageBox::Cancel);
        QPushButton *defaultBtn = box.addButton(tr("Default"), QMessageBox::ResetRole);
        box.setDefaultButton(okBtn);

        box.exec();

        qreal size;
        if (box.clickedButton() == defaultBtn) {
            size = DEFAULTFONTSIZE;
        } else if (box.clickedButton() == okBtn) {
            size = spin->value();
        } else {
            return;   // Cancel
        }

        FONTSIZE = size;
        settings.setValue("ui/fontSize", size);
        settings.sync();

        QFont f = qApp->font();
        f.setPointSizeF(size);
        qApp->setFont(f);
        qApp->setStyleSheet(qApp->styleSheet());
    });


    //
    settingsMenu->addSeparator();
    QAction* resetAction = settingsMenu->addAction("&Reset Settings");
    connect(resetAction, &QAction::triggered, this, [this]() {
        if (QMessageBox::question(this, tr("Reset Settings"),
                tr("Delete all settings and restart?")) == QMessageBox::Yes) {
            QSettings s;
            s.clear();
            s.sync();

            if (QProcess::startDetached(QCoreApplication::applicationFilePath(),
                                        QStringList())) {
                QTimer::singleShot(0, qApp, &QCoreApplication::quit);
            }
        }
    });
    //settingsMenu->addAction("&Preferences", this, &MainWindow::onOpenPreferences);

    // Tools Menu (create if it doesn't exist)
    QMenu* toolsMenu = menuBar()->addMenu("&Tools");

    // Add custom spread designer action
    QAction* createSpreadAction = toolsMenu->addAction("&Custom Spread Designer");
    connect(createSpreadAction, &QAction::triggered, this, &MainWindow::onCreateCustomSpreadClicked);
    toolsMenu->addSeparator();
    // ImportDeck
    QAction* importDeckAction = toolsMenu->addAction("&Import Deck");
    connect(importDeckAction, &QAction::triggered, this, &MainWindow::orderDeck);
    toolsMenu->addSeparator();

    //import physical spread
    QAction* importSpreadAction = toolsMenu->addAction("Import Physicaal Spread");
    connect(importSpreadAction, &QAction::triggered, this, &MainWindow::onImportPhysicalSpread);

    toolsMenu->addSeparator();
    QAction* shuffleAction = new QAction("&Shuffle Deck", this);
    shuffleAction->setShortcut(QKeySequence("Ctrl+Shift+H"));
    connect(shuffleAction, &QAction::triggered, this, [this]{
        dockControls->getShuffleButton()->click();
    });
    toolsMenu->addAction(shuffleAction);
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

    connect(journalDialog, &JournalDialog::loadReadingRequested,
            this, &MainWindow::loadReading);

    setupShareButton();

    connect(rssDialog, &RssNotificationDialog::newContentAvailable,
                this, [this](bool hasNew){

            if (!rssAction) return;

            if (hasNew) {
                rssAction->setIcon(QIcon(":/resources/icons/rss-green.svg"));
                // rssAction->setIcon(QIcon(":/icons/rss-red.svg"));
            } else {
                rssAction->setIcon(QIcon(":/resources/icons/rss.svg"));

            }


        });
}

MainWindow::~MainWindow()
{
    // The OpenGL widget might not have a proper parent
    QOpenGLWidget* glWidget = qobject_cast<QOpenGLWidget*>(centralView->viewport());
    if (glWidget && glWidget->parent() == nullptr) {
        delete glWidget;
    }
    if(tarotScene) delete tarotScene;
}



void MainWindow::createDocks() {

    // ---------------------------------------------------------------
    // Left dock — Decks and Spreads
    // ---------------------------------------------------------------
    leftDock = new QDockWidget(this);
    //leftDock->setFeatures(QDockWidget::DockWidgetClosable);

    leftDock->setAllowedAreas(Qt::LeftDockWidgetArea);

    // Custom title bar
    QLabel *leftDockTitle = new QLabel("Decks and Spreads", leftDock);
    //leftDockTitle->setFont(qApp->font());
    leftDockTitle->setAlignment(Qt::AlignCenter);
    leftDockTitle->setStyleSheet("QLabel { color: gold; background: transparent; padding: 4px; }");
    leftDock->setTitleBarWidget(leftDockTitle);

    // Content
    dockControls = new DockControls(this);
    leftDock->setWidget(dockControls);

    addDockWidget(Qt::LeftDockWidgetArea, leftDock);


    // ---------------------------------------------------------------
    // Right dock — Card Meanings
    // ---------------------------------------------------------------
    rightDock = new QDockWidget(this);
    //rightDock->setFeatures(QDockWidget::DockWidgetClosable);

    rightDock->setAllowedAreas(Qt::RightDockWidgetArea);

    // Custom title bar
    QLabel *rightDockTitle = new QLabel("Card Meanings", rightDock);
    //rightDockTitle->setFont(qApp->font());
    rightDockTitle->setAlignment(Qt::AlignCenter);
    rightDockTitle->setStyleSheet("QLabel { color: gold; background: transparent; padding: 4px; }");
    rightDock->setTitleBarWidget(rightDockTitle);

    // Content container
    QWidget *rightDockWidget = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightDockWidget);

    displayFullDeckButton = new QPushButton("Display Full Deck", this);
    rightLayout->addWidget(displayFullDeckButton);

    // Set a Question button
    openQuestionDialogButton = new QPushButton("Set a Question", this);
    openQuestionDialogButton->setToolTip("Pose a question to be forwarded to AI");
    connect(openQuestionDialogButton, &QPushButton::clicked,
            this, &MainWindow::onSetQuestion);
    rightLayout->addWidget(openQuestionDialogButton);

    // Journal button
    openJournalButton = new QPushButton("Journal", this);
    openJournalButton->setToolTip("Open the journal");
    connect(openJournalButton, &QPushButton::clicked, this, [this]() {
        journalDialog->refreshForDate(QDate::currentDate());
        journalDialog->show();
    });
    rightLayout->addWidget(openJournalButton);

    // Switch Deck button
    switchDeckButton = new QPushButton("Switch Deck", this);
    switchDeckButton->setToolTip("See your spread in different deck");
    connect(switchDeckButton, &QPushButton::clicked,
            this, &MainWindow::onShowInOtherDeck);
    rightLayout->addWidget(switchDeckButton);

    // Extra Card button
    oneMoreButton = new QPushButton("Extra Card", this);
    oneMoreButton->setToolTip(
        "Extra Card OR Significator Assigner\n\n"
        "Keep open while using - minimize/maximize via title bar.\n"
        "• Draw extra card → just open (If you need final or explanatory cards)\n"
        "• Set significator → use dropdown before dealing\n\n"
        "Use * button to set window title (e.g., 'Clarifies The Moon' or 'Significator')"
        );
    connect(oneMoreButton, &QPushButton::clicked, this, [this]() {
        bool allowReversed = dockControls->allowReversed->isChecked();
        QVector<CardLoader::CardData> randomCard =
            tarotScene->getCardLoader().getRandomCards(1, allowReversed);

        if (!randomCard.isEmpty()) {
            tarotScene->showExtraCardPopup(randomCard[0].number, randomCard[0].reversed);
        }
    });
    rightLayout->addWidget(oneMoreButton);

    //

    clearButton = new QPushButton("Clear Cards", this);

    rightLayout->addWidget(clearButton);
    //


    // Meaning display
    meaningDisplay = new MeaningDisplay(this);
    rightLayout->addWidget(meaningDisplay, 1);

    rightDock->setWidget(rightDockWidget);

    addDockWidget(Qt::RightDockWidgetArea, rightDock);


    // ---------------------------------------------------------------
    // Signal connections
    // ---------------------------------------------------------------
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
    //QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString dataLocation = getLocalDataDirPath();;

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
    json["deckName"] = dockControls->getDeckSelector()->currentText();
    //
    // save the custom spread name
    if (tarotScene->getCurrentSpreadType() == TarotScene::Custom) {
        json["customSpreadName"] = currentCustomSpreadName;
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

    // ========== ADD JOURNAL ENTRY ==========
    // Get spread type as string
    QString spreadTypeStr;
    switch (tarotScene->getCurrentSpreadType()) {
        case TarotScene::CelticCross:
            spreadTypeStr = "Celtic Cross";
            break;
        case TarotScene::ThreeCard:
            spreadTypeStr = "Three Card";
            break;
        case TarotScene::SingleCard:
            spreadTypeStr = "Single Card";
            break;
        case TarotScene::Horseshoe:
            spreadTypeStr = "Horseshoe Spread";
            break;
        case TarotScene::ZodiacSpread:
            spreadTypeStr = "Zodiac Spread";
            break;
        case TarotScene::Custom:
            spreadTypeStr = currentCustomSpreadName;
            break;
        default:
            spreadTypeStr = "Unknown";
            break;
    }

    // Get deck name (if you have this info somewhere)
    QString deckName = dockControls->getDeckSelector()->currentText();

    // Add entry to journal (empty notes, just the reading)
    JournalManager::instance().addEntry(spreadTypeStr, deckName, fileName, "");
    // =====================================

    QMessageBox::information(this, "Save Successful",
                             "Reading saved successfully to " + fileName);
}


void MainWindow::onLoadReading() {
    // Get the proper data location for the application
    //QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString dataLocation = getLocalDataDirPath();;

    if (dataLocation.isEmpty()) {
        dataLocation = QDir::homePath() + "/.local/share/TaroCaster";
    }

    // Use the data location if it exists, otherwise fall back to home directory
    QDir dir(dataLocation);
    QString loadDir = dir.exists() ? dataLocation : QDir::homePath();


    QString fileName = QFileDialog::getOpenFileName(this, "Load Reading",
                                                    loadDir,
                                                    "Tarot Readings (*.tarot)");
    if (fileName.isEmpty()) {
        return;
    }

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

    // ========== DECK SWITCHING ==========
    if (json.contains("deckName")) {
        QString savedDeckName = json["deckName"].toString();
        int deckIndex = dockControls->getDeckSelector()->findText(savedDeckName);

        if (deckIndex != -1) {
            // Saved deck exists in combobox
            if (dockControls->getDeckSelector()->currentText() != savedDeckName) {
                QMessageBox::StandardButton reply = QMessageBox::question(
                    this,
                    "Switch Deck",
                    QString("This reading was saved using the deck:\n\n\"%1\"\n\n"
                            "Your currently selected deck is:\n\n\"%2\"\n\n"
                            "Switch to the saved deck before loading?")
                        .arg(savedDeckName)
                        .arg(dockControls->getDeckSelector()->currentText()),
                    QMessageBox::Yes | QMessageBox::No
                );

                if (reply == QMessageBox::Yes) {
                    // Change the combobox
                    dockControls->getDeckSelector()->setCurrentIndex(deckIndex);
                    // This will trigger DockControls to emit deckLoaded signal
                    // which calls onDeckLoaded and loads the deck automatically
                }
            }
        } else {
            // Saved deck not found
            QMessageBox::warning(
                this,
                "Deck Not Found",
                QString("This reading was saved using the deck:\n\n\"%1\"\n\n"
                        "This deck is not available in your current deck list.\n"
                        "Cards will be displayed using your current deck.")
                    .arg(savedDeckName)
            );
        }
    }
    // ===================================

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


void MainWindow::loadReading(const QString& filename) {
    // Get the proper data location for the application
    QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dataLocation.isEmpty()) {
        dataLocation = QDir::homePath() + "/.local/share/TaroCaster";
    }

    // Use the data location if it exists, otherwise fall back to home directory
    QDir dir(dataLocation);
    QString loadDir = dir.exists() ? dataLocation : QDir::homePath();


    QString fileName = filename;
    if (fileName.isEmpty()) {
        return;
    }

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


    // ========== DECK SWITCHING ==========
    if (json.contains("deckName")) {
        QString savedDeckName = json["deckName"].toString();
        int deckIndex = dockControls->getDeckSelector()->findText(savedDeckName);

        if (deckIndex != -1) {
            // Saved deck exists in combobox
            if (dockControls->getDeckSelector()->currentText() != savedDeckName) {
                QMessageBox::StandardButton reply = QMessageBox::question(
                    this,
                    "Switch Deck",
                    QString("This reading was saved using the deck:\n\n\"%1\"\n\n"
                            "Your currently selected deck is:\n\n\"%2\"\n\n"
                            "Switch to the saved deck before loading?")
                        .arg(savedDeckName)
                        .arg(dockControls->getDeckSelector()->currentText()),
                    QMessageBox::Yes | QMessageBox::No
                );

                if (reply == QMessageBox::Yes) {
                    // Change the combobox
                    dockControls->getDeckSelector()->setCurrentIndex(deckIndex);
                    // This will trigger DockControls to emit deckLoaded signal
                    // which calls onDeckLoaded and loads the deck automatically
                }
            }
        } else {
            // Saved deck not found
            QMessageBox::warning(
                this,
                "Deck Not Found",
                QString("This reading was saved using the deck:\n\n\"%1\"\n\n"
                        "This deck is not available in your current deck list.\n"
                        "Cards will be displayed using your current deck.")
                    .arg(savedDeckName)
            );
        }
    }
    // ===================================


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

void MainWindow::onShowInOtherDeck()
{
    if (tarotScene->getCurrentCards().isEmpty()) {
        QMessageBox::warning(this, "No Spread", "No spread is currently displayed.");
        return;
    }

    // Create non-modal dialog
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("Show in Other Deck");
    dialog->setMinimumSize(300, 120);
    dialog->setModal(false);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout* layout = new QVBoxLayout(dialog);

    QLabel* label = new QLabel("Select a different deck from the dropdown, then click Show:");
    layout->addWidget(label);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* showButton = new QPushButton("Show");
    QPushButton* cancelButton = new QPushButton("Cancel");
    buttonLayout->addWidget(showButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    connect(showButton, &QPushButton::clicked, [this, dialog]() {
        // Just redraw with whatever deck is currently selected
        if (!tarotScene->getCurrentCards().isEmpty()) {
            tarotScene->redrawCurrentSpread();
        }
        dialog->close();
    });

    connect(cancelButton, &QPushButton::clicked, dialog, &QDialog::close);

    dialog->show();
}

void MainWindow::onImportPhysicalSpread()
{
    ImportPhysicalDialog* dialog = new ImportPhysicalDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

//sharing

void MainWindow::setupShareButton()
{
    // Create the share button
    QPushButton *shareButton = new QPushButton(this);
    shareButton->setIcon(QIcon(":/resources/icons-white/share-2.svg"));
    shareButton->setToolTip("Share this spread");
    shareButton->setFlat(true);
    shareButton->setFixedSize(32, 32);

    /*
    shareButton->setStyleSheet(R"(
        QPushButton {
            border: none;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: rgba(255, 255, 255, 0.2);
        }
        QPushButton:pressed {
            background-color: rgba(255, 255, 255, 0.3);
        }
    )");
    */

    // Add to menubar corner
    QMenuBar *menuBar = this->menuBar();
    if (menuBar) {
        menuBar->setCornerWidget(shareButton, Qt::TopRightCorner);
    }

    connect(shareButton, &QPushButton::clicked, this, &MainWindow::onShareClicked);
}

void MainWindow::onShareClicked()
{
    // Capture the spread
    QRectF sceneRect = tarotScene->sceneRect();
    QPixmap screenshot(sceneRect.size().toSize());
    screenshot.fill(Qt::transparent);

    QPainter painter(&screenshot);
    tarotScene->render(&painter, screenshot.rect(), sceneRect);
    painter.end();

    // Add watermark
    QPainter watermarkPainter(&screenshot);
    watermarkPainter.setPen(Qt::NoPen);
    watermarkPainter.setPen(QPen(QColor(255, 255, 255, 180), 2));
    watermarkPainter.setFont(QFont("Arial", 20, QFont::Bold));
    watermarkPainter.drawText(screenshot.rect(), Qt::AlignBottom | Qt::AlignRight,
                              "  Created with TarotCaster  ");
    watermarkPainter.end();
    // Build the text once
    QString shareText = QString("My %1 reading with the %2 deck!\n")
                        .arg(g_currentSpreadName)
                        .arg(g_currentDeckName);

    // Copy watermarked image to clipboard
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setImage(screenshot.toImage());

    // Create and show dialog
    SocialShareDialog *dialog = new SocialShareDialog(shareText, screenshot, m_socialShare, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setModal(false);
    dialog->show();
}



void MainWindow::showAIConfigGuide()
{
    QDialog dialog(this);
    dialog.setWindowTitle("AI Interpretation Guide");
    dialog.resize(600, 500);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QTextEdit *textEdit = new QTextEdit(&dialog);
    textEdit->setReadOnly(true);
    textEdit->setHtml(
        "<h2>AI-Powered I-Ching Interpretations</h2>"

        "<p>The app can use AI models to provide rich, contextual interpretations of your divinations. "
        "You can connect to various AI providers by configuring them in the Model Selector.</p>"

        "<h3>Getting Started:</h3>"
        "<ol>"
        "<li><b>Open AI Model Selector:</b> Tools → AI Model Selector</li>"
        "<li><b>Add a new model:</b> Click 'Add' and fill in the details</li>"
        "<li><b>Set as active:</b> Select the model and click 'Set Active'</li>"
        "<li><b>Get interpretations:</b> Complete a hexagram and click 'Get AI Interpretation'</li>"
        "</ol>"

        "<h3>Compatible Providers (OpenAI-compatible API format):</h3>"
        "<table width='100%' border='1' cellpadding='5'>"
        "<tr><th>Provider</th><th>Endpoint</th><th>Example Model</th><th>API Key</th></tr>"

        "<tr><td><b>Groq</b></td>"
        "<td><code>https://api.groq.com/openai/v1/chat/completions</code></td>"
        "<td><code>openai/gpt-oss-120b</code></td>"
        "<td>gsk_... (free tier)</td></tr>"

        "<tr><td><b>Mistral</b></td>"
        "<td><code>https://api.mistral.ai/v1/chat/completions</code></td>"
        "<td><code>mistral-medium</code></td>"
        "<td>Free trial</td></tr>"


        "<tr><td><b>Gemini</b></td>"
        "<td><code>https://generativelanguage.googleapis.com/v1beta/openai/chat/completions</code></td>"
        "<td><code>gemini-3.5-flash</code></td>"
        "<td>Free trial</td></tr>"


        "<tr><td><b>OpenAI</b></td>"
        "<td><code>https://api.openai.com/v1/chat/completions</code></td>"
        "<td><code>gpt-4</code> or <code>gpt-3.5-turbo</code></td>"
        "<td>Paid access</td></tr>"

        "<tr><td><b>Ollama (local)</b></td>"
        "<td><code>http://localhost:11434/v1/chat/completions</code></td>"
        "<td><code>llama3</code> or <code>mistral</code></td>"
        "<td><i>None</i></td></tr>"

        "<tr><td><b>Together AI</b></td>"
        "<td><code>https://api.together.xyz/v1/chat/completions</code></td>"
        "<td><code>mistralai/Mixtral-8x7B-Instruct</code></td>"
        "<td>Required</td></tr>"

        "<tr><td><b>DeepSeek</b></td>"
        "<td><code>https://api.deepseek.com/v1/chat/completions</code></td>"
        "<td><code>deepseek-chat</code></td>"
        "<td>Required</td></tr>"
        "</table>"

        "<h3>Configuration Tips:</h3>"
        "<ul>"
        "<li><b>Friendly Name:</b> Any name to identify this config (e.g., 'My Groq Llama')</li>"
        "<li><b>Provider:</b> Just for reference (e.g., 'Groq', 'OpenAI')</li>"
        "<li><b>Endpoint URL:</b> The full API URL from the table above</li>"
        "<li><b>API Key:</b> Get from provider's website (except Ollama)</li>"
        "<li><b>Model Name:</b> The specific model identifier from the provider</li>"
        "<li><b>Temperature:</b> Keep at 0.7 for balanced interpretations</li>"
        "<li><b>Max Tokens:</b> 4096 is usually sufficient</li>"
        "</ul>"

        "<h3>Finding Model Names and Endpoints:</h3>"
        "<p>If you're unsure about which model to use or need the exact endpoint URL:</p>"
        "<ul>"
        "<li><b>Ask AI assistants</b> like ChatGPT, Claude: "
        "\"What's the API endpoint and available models for [Provider]?\"</li>"
        "<li><b>Check provider documentation</b> - most have clear API reference pages</li>"
        "<li><b>Search online:</b> '[Provider] API documentation'</li>"
        "</ul>"

        "<h3>Recommended Settings by Provider:</h3>"
        "<ul>"
        "<li><b>Groq:</b> <code>openai/gpt-oss-120b</code> for best quality</li>"
        "<li><b>Mistral:</b> <code>mistral-medium</code> works well</li>"
        "<li><b>Gemini(Google):</b> <code>gemini-3.5-flash</code> works well</li>"

        "<li><b>OpenAI:</b> <code>gpt-4</code> best results, <code>gpt-3.5-turbo</code> faster/cheaper</li>"
        "<li><b>Ollama:</b> Install Ollama first, then pull <code>llama3</code> or <code>mistral</code></li>"
        "</ul>"

        "<h3 style='color: #ff6b6b;'>Important Notes:</h3>"
        "<ul>"
        "<li><b>API keys are stored locally</b> in your system's secure settings</li>"
        "<li><b>Not compatible:</b> Claude (Anthropic) - different API formats</li>"
        "<li><b>Restart app</b> after configuring your first model</li>"
        "<li><b>Hexagram data and question</b> are sent to the configured AI service</li>"
        "</ul>"
    );

    layout->addWidget(textEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *openConfigButton = new QPushButton("Open Model Selector", &dialog);
    QPushButton *closeButton = new QPushButton("Close", &dialog);

    buttonLayout->addStretch();
    buttonLayout->addWidget(openConfigButton);
    buttonLayout->addWidget(closeButton);

    layout->addLayout(buttonLayout);

    connect(openConfigButton, &QPushButton::clicked, &dialog, [this, &dialog]() {
        dialog.accept();
        ModelSelectorDialog dlg(this);
        dlg.exec();
        //aiManager->loadActiveModel();
    });

    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
}
