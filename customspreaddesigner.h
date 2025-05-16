#ifndef CUSTOMSPREADDESIGNER_H
#define CUSTOMSPREADDESIGNER_H

#include <QDialog>
#include <QPointF>
#include <QVector>
#include <QString>
#include<QDialog>

class QLineEdit;
class QTextEdit;
class QPushButton;
class QListWidget;
class QLabel;
class TarotScene;
class QVBoxLayout;

class CustomSpreadDesigner : public QDialog {
    Q_OBJECT

public:
    struct CardPosition {
        int number;
        QString name;
        QString significance;
        QPointF position;
    };

    CustomSpreadDesigner(TarotScene* scene, QWidget* parent = nullptr);
    ~CustomSpreadDesigner();

private slots:
    void addCard();
    void saveSpread();
    void positionSelected(int index);
    void manageCustomSpreads();


private:
    void setupUI();
    void clearPositionFields();
    void updatePositionList();
    bool validateCurrentPosition();
    bool validateSpread();

    TarotScene* m_scene;

    // UI elements
    QLineEdit* m_spreadNameEdit;
    QTextEdit* m_spreadDescriptionEdit;
    QLabel* m_positionNumberLabel;
    QLineEdit* m_positionNameEdit;
    QTextEdit* m_positionSignificanceEdit;
    QPushButton* m_addCardButton;
    QPushButton* m_saveSpreadButton;
    QListWidget* m_positionsList;

    // Data
    QVector<CardPosition> m_positions;
    int m_currentPositionNumber;
    int m_selectedPositionIndex;
    void updatePositionsFromScene();
    void highlightSelectedCard(int positionNumber);
    //managespreads
    QPushButton* m_manageSpreadsButton;

};




#endif // CUSTOMSPREADDESIGNER_H
