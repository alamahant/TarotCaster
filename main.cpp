#include "mainwindow.h"

#include <QApplication>
#include <QSettings>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("TarotCaster");  // Instead of "Your Organization"
    QCoreApplication::setApplicationName("TarotCaster");
    MainWindow w;
    w.show();
    return a.exec();
}
