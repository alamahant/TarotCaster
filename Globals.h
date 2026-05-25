#ifndef GLOBALS_H
#define GLOBALS_H
#include<QMap>
#include<QString>

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

#endif // GLOBALS_H
