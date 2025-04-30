#ifndef DOCKCONTROLS_H
#define DOCKCONTROLS_H

#include <QWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGroupBox>
#include "cardloader.h"
#include<QTimer>
#include<QRandomGenerator>
#include<QTextEdit>
#include<QLabel>
#include<QStandardPaths>

class DockControls : public QWidget
{
    Q_OBJECT
public:
    explicit DockControls(QWidget *parent = nullptr);

    // Getters for the controls
    QComboBox* getDeckSelector() const { return deckSelector; }
    QComboBox* getSpreadSelector() const { return spreadSelector; }
    QPushButton* getShuffleButton() const { return shuffleButton; }
    QPushButton* getDealButton() const { return dealButton; }
    QPushButton* dealButton;
    QPushButton* clearButton;
    QCheckBox* allowReversed;
    QComboBox* spreadSelector;
    static QRandomGenerator randomGen;

signals:
    void deckLoaded(CardLoader* loader);
    void reversedCardsToggled(bool allowed);
    void spreadTypeChanged(const QString& spreadType);
    void dealRequested();
    void displayFullDeckRequested();
    void getReadingRequested();
private slots:
    void onDeckSelected(const QString& deckName);
    void onReversedToggled(bool allowed);
    void onDisplayFullDeckClicked();

public slots:
    void onDealClicked();
    void collectMouseData();
    void toggleShuffle();
private:
    CardLoader* cardLoader;
    QComboBox* deckSelector;
    QPushButton* shuffleButton;
    bool isShuffling = false;
    QTimer* mouseTimer;
    QVector<qint64> entropyData;
    void generateSeed();
    QPushButton* displayFullDeckButton;

public:
    QPushButton* getReadingButton;
    QTextEdit* readingDisplay;
private:
    void loadAvailableDecks();

};

#endif // DOCKCONTROLS_H
