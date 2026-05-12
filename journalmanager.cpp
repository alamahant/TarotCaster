#include "journalmanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include"Globals.h"

JournalManager& JournalManager::instance()
{
    static JournalManager manager;
    return manager;
}

JournalManager::JournalManager(QObject* parent)
    : QObject(parent)
{
    load();
}

void JournalManager::addEntry(const QString& spreadType, const QString& deckName, const QString& filePath, const QString& notes)
{
    JournalEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.spreadType = spreadType;
    entry.deckName = deckName;
    entry.filePath = filePath;
    entry.notes = notes;

    m_entries.append(entry);
    save();
}

QVector<JournalEntry> JournalManager::getEntries() const
{
    return m_entries;
}

QVector<JournalEntry> JournalManager::getEntriesForDate(const QDate& date) const
{
    QVector<JournalEntry> result;
    for (const JournalEntry& entry : m_entries) {
        if (entry.timestamp.date() == date) {
            result.append(entry);
        }
    }
    return result;
}

void JournalManager::clearAll()
{
    m_entries.clear();
    save();
}


void JournalManager::save()
{
    QJsonArray entriesArray;
    for (const JournalEntry& entry : m_entries) {
        QJsonObject obj;
        obj["timestamp"] = entry.timestamp.toString(Qt::ISODate);
        obj["spreadType"] = entry.spreadType;
        obj["deckName"] = entry.deckName;
        obj["filePath"] = entry.filePath;
        obj["notes"] = entry.notes;
        entriesArray.append(obj);
    }

    QJsonDocument doc(entriesArray);
    QFile file(getFilePath());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void JournalManager::load()
{
    QFile file(getFilePath());
    if (!file.open(QIODevice::ReadOnly)) return;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isArray()) return;

    m_entries.clear();
    for (const QJsonValue& val : doc.array()) {
        QJsonObject obj = val.toObject();
        JournalEntry entry;
        entry.timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);
        entry.spreadType = obj["spreadType"].toString();
        entry.deckName = obj["deckName"].toString();
        entry.filePath = obj["filePath"].toString();
        entry.notes = obj["notes"].toString();
        m_entries.append(entry);
    }
}

QString JournalManager::getFilePath() const
{
    QString path = getJournalDirPath();


    return path + "/journal.json";
}

void JournalManager::addEntry(const JournalEntry& entry)
{
    m_entries.append(entry);
    save();
}


void JournalManager::deleteEntriesForDate(const QDate& date)
{
    for (int i = m_entries.size() - 1; i >= 0; --i) {
        if (m_entries[i].timestamp.date() == date) {
            m_entries.removeAt(i);
        }
    }
    save();
}
