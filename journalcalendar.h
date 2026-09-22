#ifndef JOURNALCALENDAR_H
#define JOURNALCALENDAR_H

#include <QCalendarWidget>
#include <QDate>
#include <QSet>

class JournalCalendar : public QCalendarWidget
{
    Q_OBJECT

public:
    explicit JournalCalendar(QWidget* parent = nullptr);

    void setDatesWithEntries(const QSet<QDate>& dates);

protected:
    // Qt6 signature: 3 arguments, QDate (by value), NOT const QDate&, NOT bool
    void paintCell(QPainter* painter, const QRect& rect, QDate date) const override;

private:
    QSet<QDate> m_datesWithEntries;
};

#endif
