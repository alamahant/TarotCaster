#include "mainwindow.h"

#include <QApplication>
#include <QSettings>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("");
    QCoreApplication::setApplicationName("TarotCaster");
    a.setWindowIcon(QIcon(":/resources/app-blue.png"));


    QFile styleFile(":/resources/tarotcaster.css");

    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&styleFile);
        a.setStyleSheet(stream.readAll());
        styleFile.close();
    } else {
        qWarning() << "Failed to load stylesheet:" << styleFile.errorString();
    }


    MainWindow w;
    w.show();
    return a.exec();
}
