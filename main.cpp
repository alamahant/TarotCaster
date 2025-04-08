#include "mainwindow.h"

#include <QApplication>
#include <QSettings>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("TarotCaster");  // Instead of "Your Organization"
    QCoreApplication::setApplicationName("TarotCaster");
    a.setWindowIcon(QIcon(":/resources/favicon-32x32.png"));
    MainWindow w;
    w.show();
    return a.exec();
}
