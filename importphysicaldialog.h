// importphysicaldialog.h
#ifndef IMPORTPHYSICALDIALOG_H
#define IMPORTPHYSICALDIALOG_H

#include <QDialog>
#include <QMap>
#include <QVector>
#include <QJsonObject>
#include<QListWidget>
#include<QLabel>
#include<QCheckBox>

struct AssignedCard {
    int number;
    int position;
    bool reversed;
};

class ImportPhysicalDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportPhysicalDialog(QWidget *parent = nullptr);
    ~ImportPhysicalDialog();

private slots:
    void onAssignClicked();
    void onSaveClicked();

private:
    void setupUI();
    void populateCardList();
    void loadSpreadPositions();
    void updatePositionList();
    QString getCardName(int number) const;
    QJsonObject buildJson() const;
    QString getCurrentTimestamp() const;
    void showErrorMessage(const QString& message);

    
    // Spread data (loaded from globals and custom_spreads.json)
    QStringList m_positionNames;
    int m_spreadType;
    QString m_deckName;
    
    // Assigned cards
    QVector<AssignedCard> m_assignedCards;
    
    // UI elements
    QListWidget* m_cardListWidget;
    QListWidget* m_positionListWidget;
    QCheckBox* m_reversedCheckBox;
    QPushButton* m_assignButton;
    QPushButton* m_saveButton;
    QPushButton* m_cancelButton;
    QLabel* m_selectedPositionLabel;
    
    int m_currentSelectedPosition;
};

#endif // IMPORTPHYSICALDIALOG_H
