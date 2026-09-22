#include "mainwindow.h"

#include <QApplication>
#include <QSettings>
#include"Globals.h"
#include<QDir>
#include<QDebug>
#include<QFile>
#include<QCoreApplication>
#include<QSettings>
#include<QStyleFactory>

int main(int argc, char *argv[])
{

    QCoreApplication::setOrganizationName("Alamahant");
    QCoreApplication::setApplicationName("TarotCaster");
    QCoreApplication::setApplicationVersion("1.3.0");
    QDir().mkpath(getLocalDataDirPath());
    QDir().mkpath(getUserDecksDirPath());
    QDir().mkpath(getUnorderedDecksDirPath());
    QDir().mkpath(getJournalDirPath());
    QDir().mkpath(getSharesDirPath());



    //QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QSettings s;
    double factor = s.value("ui/scaleFactor", 1.0).toDouble();
    qputenv("QT_SCALE_FACTOR", QByteArray::number(factor));
    FONTSIZE = s.value("ui/fontSize", DEFAULTFONTSIZE).toReal();

    QApplication a(argc, argv);

#ifdef Q_OS_WIN
    a.setStyle(QStyleFactory::create("Fusion"));

    QPalette lightPalette;
    lightPalette.setColor(QPalette::Window, Qt::white);
    lightPalette.setColor(QPalette::WindowText, Qt::black);
    lightPalette.setColor(QPalette::Base, Qt::white);
    lightPalette.setColor(QPalette::Text, Qt::black);
    lightPalette.setColor(QPalette::Button, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::ButtonText, Qt::black);
    lightPalette.setColor(QPalette::Highlight, QColor(0, 120, 215));
    lightPalette.setColor(QPalette::HighlightedText, Qt::white);

    a.setPalette(lightPalette);
    a.setStyleSheet("QLineEdit { placeholder-text-color: #999999; }");

#endif


    a.setWindowIcon(QIcon(":/resources/app-blue.png"));

    QFile styleFile(":/resources/tarotcaster.css");

    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&styleFile);
        a.setStyleSheet(stream.readAll());
        styleFile.close();
    } else {
        qWarning() << "Failed to load stylesheet:" << styleFile.errorString();
    }

    if (FONTSIZE > 0.0) {
        QFont appFont = a.font();
        appFont.setPointSizeF(FONTSIZE);
        a.setFont(appFont);
    } else {
        FONTSIZE = DEFAULTFONTSIZE;
    }

    MainWindow w;
    w.show();

    return a.exec();
}
