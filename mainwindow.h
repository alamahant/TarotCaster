#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QDockWidget>
#include "tarotscene.h"
#include<QProgressBar>
#include<QInputDialog>
#include<QLineEdit>
#include<QFileDialog>
#include<QMessageBox>
#include<QMenu>
#include<QMenuBar>

#include "meaningdisplay.h"
#include "tarotscene.h"
#include "dockcontrols.h"
#include"mistralapi.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void testCards();
    void clearMeaningDisplay();

private slots:
    void setupUI();
    void createDocks();
    void onDeckLoaded(CardLoader* loader);
    void onReversedCardsToggled(bool allowed);

public slots:
   void showCardMeaning(int cardNumber);
    void onDealClicked();
   void onGetReadingClicked();

private:
    QGraphicsView *centralView;
    QDockWidget *leftDock;   // For spread selection
    QDockWidget *rightDock;  // For card meanings
    QMap<int, CardMeaning> cardMeanings;  // Store our loaded meanings
    TarotScene* tarotScene;
    DockControls* dockControls;
    MeaningDisplay* meaningDisplay;

private:
    MistralAPI* mistralApi;

private slots:
    //void onReadingPromptReady(const QString& prompt);
    void onReadingReady(const QString& reading);
    void onApiError(const QString& errorMessage);

//save load
private slots:
    void onSaveReading();
    void onLoadReading();

private slots:
    void onEditApiKey();
};
#endif // MAINWINDOW_H
