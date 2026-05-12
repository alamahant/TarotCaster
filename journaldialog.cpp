#include "journaldialog.h"
#include "journalmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCalendarWidget>
#include <QTextBrowser>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QDateTime>
#include<QHeaderView>
#include<QTableView>
#include<QSplitter>
#include<QMessageBox>

JournalDialog::JournalDialog(QWidget* parent)
    : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose, false);  // Ensure it doesn't delete on close
    setupUI();
    refreshForDate(QDate::currentDate());
}



void JournalDialog::setupUI()
{
    setWindowTitle("Journal");
    setMinimumSize(800, 800);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // ========== QSplitter for resizable sections ==========
    QSplitter* splitter = new QSplitter(Qt::Vertical, this);
    splitter->setStyleSheet(
        "QSplitter::handle {"
        "   background-color: #cccccc;"
        "   height: 3px;"
        "}"
    );
    // Top section: Calendar
    m_calendar = new JournalCalendar(this);
    m_calendar->setObjectName("tibetanCalendar");
    m_calendar->setGridVisible(true);
    m_calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    m_calendar->setStyleSheet(
        "QCalendarWidget QAbstractItemView {"
        "   color: #666666;"
        "}"
    );
    connect(m_calendar, &QCalendarWidget::selectionChanged, this, [this]() {
        onDateSelected(m_calendar->selectedDate());
    });

    // Create a widget to hold calendar with proper sizing
    QWidget* calendarContainer = new QWidget(this);
    QVBoxLayout* calendarLayout = new QVBoxLayout(calendarContainer);
    calendarLayout->setContentsMargins(0, 0, 0, 0);
    calendarLayout->addWidget(m_calendar);
    splitter->addWidget(calendarContainer);

    // Bottom section: Entries display + Add entry
    QWidget* entriesContainer = new QWidget(this);
    QVBoxLayout* entriesLayout = new QVBoxLayout(entriesContainer);
    entriesLayout->setContentsMargins(0, 0, 0, 0);

    // Past Entries label
    QLabel* entriesLabel = new QLabel("Past Entries:", this);
    entriesLabel->setStyleSheet("color: #ffd700; font-weight: bold; margin-top: 10px;");
    entriesLayout->addWidget(entriesLabel);

    // Entries display
    m_entriesDisplay = new QTextBrowser(this);
    m_entriesDisplay->setOpenLinks(false);  // Don't automatically open links
    m_entriesDisplay->setOpenExternalLinks(false);  // Don't open external
    m_entriesDisplay->setMinimumHeight(150);
    m_entriesDisplay->setStyleSheet("QTextBrowser { background-color: #111; color: #ccc; border: 1px solid #333; border-radius: 4px; padding: 10px; }");

    connect(m_entriesDisplay, &QTextBrowser::anchorClicked, this, &JournalDialog::onAnchorClicked);

    entriesLayout->addWidget(m_entriesDisplay);

    // Add entry section
    QLabel* newEntryLabel = new QLabel("Add Entry for Selected Date:", this);
    newEntryLabel->setStyleSheet("color: #ffd700; font-weight: bold; margin-top: 10px;");
    entriesLayout->addWidget(newEntryLabel);

    m_notesEdit = new QTextEdit(this);
    m_notesEdit->setMaximumHeight(100);
    m_notesEdit->setPlaceholderText("Write your journal notes here...");
    m_notesEdit->setStyleSheet("QTextEdit { background-color: #111; color: #ccc; border: 1px solid #333; border-radius: 4px; padding: 8px; }");
    entriesLayout->addWidget(m_notesEdit);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_saveButton = new QPushButton("Save Entry", this);
    m_saveButton->setFixedHeight(32);
    m_saveButton->setStyleSheet("QPushButton { background-color: #3a6ea5; border-radius: 4px; }");
    connect(m_saveButton, &QPushButton::clicked, this, &JournalDialog::onSaveEntry);

    m_deleteButton = new QPushButton("Delete Entry", this);
    m_deleteButton->setFixedHeight(32);
    m_deleteButton->setStyleSheet("QPushButton { background-color: #3a6ea5; border-radius: 4px; }");
    connect(m_deleteButton, &QPushButton::clicked, this, &JournalDialog::onDeleteEntry);


    m_closeButton = new QPushButton("Close", this);
    m_closeButton->setFixedHeight(32);
    m_closeButton->setStyleSheet("QPushButton { background-color: #3a3a3a; border-radius: 4px; }");
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);

    buttonLayout->addWidget(m_saveButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addWidget(m_closeButton);
    buttonLayout->addStretch();
    entriesLayout->addLayout(buttonLayout);

    splitter->addWidget(entriesContainer);

    // Set initial sizes (50% each, or adjust as needed)
    splitter->setSizes(QList<int>() << 400 << 400);

    mainLayout->addWidget(splitter);

    setStyleSheet("QDialog { background-color: #1a1a1a; } QLabel { color: #ccc; }");
}

void JournalDialog::onDateSelected(const QDate& date)
{
    refreshForDate(date);
}

void JournalDialog::refreshForDate(const QDate& date)
{
    m_currentDate = date;
    loadEntriesForDate(date);
    m_notesEdit->clear();
    updateCalendarIndicators();
}

void JournalDialog::loadEntriesForDate(const QDate& date)
{
    QVector<JournalEntry> entries = JournalManager::instance().getEntriesForDate(date);
    displayEntries(entries);
}


void JournalDialog::displayEntries(const QVector<JournalEntry>& entries)
{
    if (entries.isEmpty()) {
        m_entriesDisplay->setText("No entries for this date.");
        return;
    }

    QString html = "<table width='100%'>";
    for (const JournalEntry& entry : entries) {
        html += "<tr>";
        html += QString("<td valign='top' width='80'><b>%1</b></td>")
                .arg(entry.timestamp.toString("hh:mm"));

        html += QString("<td>");

        // Show spread type if this is a tarot reading entry
        if (!entry.spreadType.isEmpty()) {
            html += QString("<b>%1</b>").arg(entry.spreadType);
            if (!entry.deckName.isEmpty()) {
                html += QString(" (%1)").arg(entry.deckName);
            }
        }

        // Show notes if any
        if (!entry.notes.isEmpty()) {
            if (!entry.spreadType.isEmpty()) {
                html += "<br>";
            }
            html += QString("<i>%1</i>").arg(entry.notes.toHtmlEscaped());
        }

        // Add Load Reading link if this entry has a file path
        if (!entry.filePath.isEmpty()) {
            QString encodedPath = QUrl::toPercentEncoding(entry.filePath);
            html += QString("<br><a href='loadreading:%1' style='"
                "display: inline-block; "
                "background-color: #3a6ea5; "
                "color: white; "
                "padding: 4px 12px; "
                "margin-top: 8px; "
                "text-decoration: none; "
                "border-radius: 4px; "
                "font-size: 12px;"
                "'>🔮 Load Reading</a>").arg(encodedPath);
        }

        html += "</td></td>";
        html += "<tr><td colspan='2'><hr></td></tr>";
    }
    html += "</table>";

    m_entriesDisplay->setHtml(html);
}

void JournalDialog::onSaveEntry()
{
    QString notes = m_notesEdit->toPlainText().trimmed();
    if (notes.isEmpty()) {
        return;
    }

    JournalEntry entry;
    entry.timestamp = QDateTime(m_currentDate, QTime::currentTime());
    entry.spreadType = "";      // Manual entry has no spread
    entry.deckName = "";        // Manual entry has no deck
    entry.filePath = "";        // Manual entry has no file
    entry.notes = notes;

    JournalManager::instance().addEntry(entry);
    refreshForDate(m_currentDate);
}

void JournalDialog::updateCalendarIndicators()
{
    // Get all dates that have entries
    QVector<JournalEntry> allEntries = JournalManager::instance().getEntries();
    QSet<QDate> datesWithEntries;

    for (const JournalEntry& entry : allEntries) {
        datesWithEntries.insert(entry.timestamp.date());
    }

    m_calendar->setDatesWithEntries(datesWithEntries);
}

void JournalDialog::onAnchorClicked(const QUrl& url)
{
    if (url.isEmpty()) {
        return;  // Ignore clicks on plain text
    }

    if (url.scheme() == "loadreading") {
        QString filePath = QUrl::fromPercentEncoding(url.path().toUtf8());
        emit loadReadingRequested(filePath);
    }
}


void JournalDialog::onDeleteEntry()
{
    QVector<JournalEntry> entries = JournalManager::instance().getEntriesForDate(m_currentDate);

    if (entries.isEmpty()) {
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Delete Entry",
        "Delete all journal entries for " + m_currentDate.toString("yyyy-MM-dd") + "?",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        JournalManager::instance().deleteEntriesForDate(m_currentDate);
        refreshForDate(m_currentDate);
        updateCalendarIndicators();
    }
}



