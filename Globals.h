#ifndef GLOBALS_H
#define GLOBALS_H
#include<QMap>
#include<QString>

extern const QMap<QString, QString> tarotNames;
QString getUserDecksDirPath();
QString getUnorderedDecksDirPath();
QString getLocalDataDirPath();
QString getJournalDirPath();
extern bool activeModelLoaded;

#endif // GLOBALS_H
