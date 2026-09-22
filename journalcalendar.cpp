#include "journalcalendar.h"
#include <QPainter>

JournalCalendar::JournalCalendar(QWidget* parent)
    : QCalendarWidget(parent)
{
}

void JournalCalendar::setDatesWithEntries(const QSet<QDate>& dates)
{
    m_datesWithEntries = dates;
    update(); // Force a full repaint of the calendar
}

void JournalCalendar::paintCell(QPainter* painter, const QRect& rect, QDate date) const
{
    // 1. Call the base class implementation FIRST to draw the date number
    QCalendarWidget::paintCell(painter, rect, date);

    // 2. Draw our custom indicator if this date has journal entries
    if (m_datesWithEntries.contains(date)) {
        painter->save();

        // Draw a gold asterisk in the top-right corner
        painter->setPen(QColor(255, 215, 0)); // Gold color

        QFont font = painter->font();
        font.setBold(true);
        painter->setFont(font);

        int x = rect.right() - 12;
        int y = rect.top() + 12;
        painter->drawText(x, y, "*");

        painter->restore();
    }
}
