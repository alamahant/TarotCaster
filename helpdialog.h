#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QTextBrowser>
#include <QPushButton>
#include <QLabel>
#include "tarotscene.h"

class HelpDialog : public QDialog
{
    Q_OBJECT

public:
    enum DialogType {
        About,
        Instructions,
        Spreads,
        AddDecks,
        CustomSpreads,
        CustomSpreadHelp,
        ChangeLogHelp

    };

    explicit HelpDialog(DialogType type, QWidget *parent = nullptr);
    void setTarotScene(TarotScene* scene) {
        tarotScene = scene;
        // Only refresh if this is a CustomSpreads dialog
        if (tarotScene && currentType == CustomSpreads) {
            setupCustomSpreads();
        }
    }
private:
    void setupAbout();
    void setupInstructions();
    void setupSpreads();
    void setupDecks();
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QTextBrowser *contentBrowser;
    QPushButton *closeButton;
    void setupCustomSpreads();
    TarotScene* tarotScene = nullptr;
    DialogType currentType;
    void setupCustomSpreadHelp();
    void setupChangelogHelp();
};

#endif // HELPDIALOG_H
