#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QTextBrowser>
#include <QPushButton>
#include <QLabel>

class HelpDialog : public QDialog
{
    Q_OBJECT

public:
    enum DialogType {
        About,
        Instructions,
        Spreads,
        AddDecks
    };

    explicit HelpDialog(DialogType type, QWidget *parent = nullptr);

private:
    void setupAbout();
    void setupInstructions();
    void setupSpreads();
    void setupDecks();
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QTextBrowser *contentBrowser;
    QPushButton *closeButton;
};

#endif // HELPDIALOG_H
