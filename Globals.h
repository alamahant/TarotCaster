#ifndef GLOBALS_H
#define GLOBALS_H
#include<QMap>
#include<QString>
#include<QtGlobal>
#include<QGuiApplication>
#include<QScreen>


extern const QMap<QString, QString> tarotNames;

QString getUserDecksDirPath();
QString getUnorderedDecksDirPath();
QString getLocalDataDirPath();
QString getJournalDirPath();
QString getSharesDirPath();

extern bool activeModelLoaded;


extern QString g_currentSpreadName;
extern bool g_isCustomSpread;
extern QString g_currentDeckName;
extern int g_currentSpreadType;  // optional, for JSON
extern QString g_currentDeckPath;
//extern const qreal CARD_WIDTH;
//extern const qreal CARD_HEIGHT;

inline qreal getCardWidth() {
    QScreen *screen = QGuiApplication::primaryScreen();
    qreal screenWidth = screen->geometry().width();

    // Cards take about 10% of screen width (adjust as needed)
    return qMax(200.0, screenWidth * 0.10);  // Min 120px, max based on screen
}

inline qreal getCardHeight() {
    QScreen *screen = QGuiApplication::primaryScreen();
    qreal screenHeight = screen->geometry().height();
    // Cards take about 15% of screen height
    return qMax(300.0, screenHeight * 0.15);  // Min 180px
}

// Dynamic spacing based on card size
inline qreal getHorizontalSpacing() {
    return getCardWidth() * 0.27;  // 27% of card width
}

inline qreal getVerticalSpacing() {
    return getCardHeight() * 0.15;  // 15% of card height
}

#endif // GLOBALS_H
