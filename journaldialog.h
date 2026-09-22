#ifndef JOURNALDIALOG_H
#define JOURNALDIALOG_H

#include <QDialog>
#include <QDate>
#include <QVector>
#include"journalcalendar.h"
#include<QSet>

struct JournalEntry;

class QCalendarWidget;
class QTextBrowser;
class QTextEdit;
class QPushButton;

class JournalDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JournalDialog(QWidget* parent = nullptr);
    
    void refreshForDate(const QDate& date);
    void setCurrentModuleName(const QString& moduleName) { m_currentModuleName = moduleName; }
private slots:
    void onDateSelected(const QDate& date);
    void onSaveEntry();
    void onDeleteEntry();

    void onAnchorClicked(const QUrl& url);

signals:
    void loadReadingRequested(const QString& filePath);

private:
    void setupUI();
    void loadEntriesForDate(const QDate& date);
    void displayEntries(const QVector<JournalEntry>& entries);
    
    //QCalendarWidget* m_calendar;
    JournalCalendar* m_calendar;

    QTextBrowser* m_entriesDisplay;
    QTextEdit* m_notesEdit;
    QPushButton* m_saveButton;
    QPushButton* m_closeButton;
    QDate m_currentDate;
    QString m_currentModuleName;
    void updateCalendarIndicators();
    QPushButton* m_deleteButton;
};

#endif
