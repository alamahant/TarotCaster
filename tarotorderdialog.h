#ifndef TAROTORDERDIALOG_H
#define TAROTORDERDIALOG_H

#include <QDialog>

#include <QMap>
#include <QDir>
#include<QStringList>

class QComboBox;
class QLabel;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class TarotOrderDialog : public QDialog {
    Q_OBJECT
public:
    explicit TarotOrderDialog(const QString& deckPath, QWidget* parent = nullptr);

private slots:
    void assignTemporarily();
    void moveNext();
    void movePrev();
    void confirmAll();
    void cancelProcess();
    void unassignCurrent();

private:
    void loadCurrentCard();
    void updateUI();
    void backupOriginals();
    void restoreOriginals();
    void finalizeRenaming();

    // State
    QString m_deckPath;
    QStringList m_cardFiles;
    QMap<int, QString> m_assignments; // index -> cardId
    int m_currentIndex = 0;
    bool m_confirmed = false;

    // UI
    QLabel* m_imageLabel;
    QLabel* m_progressLabel;
    QComboBox* m_nameCombo;
    QPushButton* m_assignBtn;
    QPushButton* m_nextBtn;
    QPushButton* m_prevBtn;
    QPushButton* m_confirmBtn;
    QPushButton* m_cancelBtn;
    QPushButton *m_unassignBtn;
    QString detectDominantImageFormat() const;

};


#endif // TAROTORDERDIALOG_H
