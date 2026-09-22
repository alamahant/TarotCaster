#ifndef MEANINGDISPLAY_H
#define MEANINGDISPLAY_H

#include <QWidget>
#include<QTextEdit>
#include "cardmeaning.h"
class MeaningDisplay : public QTextEdit
{
    Q_OBJECT
public:
    explicit MeaningDisplay(QWidget *parent = nullptr);
    void displayMeaning(const CardMeaning& meaning);

};

#endif // MEANINGDISPLAY_H
