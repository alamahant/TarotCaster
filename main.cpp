#include "mainwindow.h"

#include <QApplication>
#include <QSettings>
#include"Globals.h"
#include<QDir>
#include<QDebug>
#include<QFile>
#include<QCoreApplication>
#include<QSettings>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("Alamahant");
    QCoreApplication::setApplicationName("TarotCaster");
    QCoreApplication::setApplicationVersion("1.2.9");
    QDir().mkpath(getLocalDataDirPath());
    QDir().mkpath(getUserDecksDirPath());
    QDir().mkpath(getUnorderedDecksDirPath());
    QDir().mkpath(getJournalDirPath());
    QDir().mkpath(getSharesDirPath());

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
