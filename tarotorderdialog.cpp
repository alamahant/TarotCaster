#include "tarotorderdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include"Globals.h"

TarotOrderDialog::TarotOrderDialog(const QString& deckPath, QWidget* parent)
    : QDialog(parent), m_deckPath(deckPath)
{
    // Setup UI
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Card Display
    m_imageLabel = new QLabel(this);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setStyleSheet("border: 2px solid gray;");
    mainLayout->addWidget(m_imageLabel);

    // Progress
    m_progressLabel = new QLabel(this);
    mainLayout->addWidget(m_progressLabel);

    // Name Selection
    m_nameCombo = new QComboBox(this);
    for (auto it = tarotNames.constBegin(); it != tarotNames.constEnd(); ++it) {
        m_nameCombo->addItem(it.value(), it.key());
    }
    mainLayout->addWidget(m_nameCombo);

    // Navigation Buttons
    QHBoxLayout* navLayout = new QHBoxLayout();
    m_prevBtn = new QPushButton("← Previous", this);
    m_assignBtn = new QPushButton("Assign Card", this);
    m_unassignBtn = new QPushButton("Undo", this);  // New button

    m_nextBtn = new QPushButton("Next →", this);
    navLayout->addWidget(m_prevBtn);
    navLayout->addWidget(m_assignBtn);
    navLayout->addWidget(m_unassignBtn);  // Added to layout

    navLayout->addWidget(m_nextBtn);
    mainLayout->addLayout(navLayout);

    // Action Buttons
    QHBoxLayout* actionLayout = new QHBoxLayout();
    m_confirmBtn = new QPushButton("Confirm All", this);
    m_cancelBtn = new QPushButton("Cancel", this);
    actionLayout->addWidget(m_confirmBtn);
    actionLayout->addWidget(m_cancelBtn);
    mainLayout->addLayout(actionLayout);

    // Connections
    connect(m_assignBtn, &QPushButton::clicked, this, &TarotOrderDialog::assignTemporarily);
    connect(m_unassignBtn, &QPushButton::clicked, this, &TarotOrderDialog::unassignCurrent);  // New connection

    connect(m_prevBtn, &QPushButton::clicked, this, &TarotOrderDialog::movePrev);
    connect(m_nextBtn, &QPushButton::clicked, this, &TarotOrderDialog::moveNext);
    connect(m_confirmBtn, &QPushButton::clicked, this, &TarotOrderDialog::confirmAll);
    connect(m_cancelBtn, &QPushButton::clicked, this, &TarotOrderDialog::cancelProcess);

    // Initialize
    backupOriginals();
    loadCurrentCard();
}


void TarotOrderDialog::loadCurrentCard() {
    QPixmap pix(m_cardFiles[m_currentIndex]);
    m_imageLabel->setPixmap(pix.scaled(400, 600, Qt::KeepAspectRatio));
    updateUI();
}
/*
void TarotOrderDialog::updateUI() {
    // Update selection
    if (m_assignments.contains(m_currentIndex)) {
        int idx = m_nameCombo->findData(m_assignments[m_currentIndex]);
        if (idx >= 0) m_nameCombo->setCurrentIndex(idx);
    } else {
        m_nameCombo->setCurrentIndex(0);
    }

    // Update buttons
    m_prevBtn->setEnabled(m_currentIndex > 0);
    m_nextBtn->setEnabled(m_currentIndex < m_cardFiles.size() - 1);
    m_assignBtn->setEnabled(true);

    // Update status
    m_progressLabel->setText(
        QString("Card %1/%2 | Assigned: %3")
            .arg(m_currentIndex + 1)
            .arg(m_cardFiles.size())
            .arg(m_assignments.size()));

    // Visual feedback
    m_imageLabel->setStyleSheet(
        m_assignments.contains(m_currentIndex)
            ? "border: 3px solid #4CAF50;"  // Green for assigned
            : "border: 2px solid gray;");   // Gray for unassigned
}
*/

void TarotOrderDialog::updateUI() {
    // Sync combo box with current card's assignment (if any)
    if (m_assignments.contains(m_currentIndex)) {
        QString assignedId = m_assignments[m_currentIndex];
        int idx = m_nameCombo->findData(assignedId);
        if (idx >= 0) {
            m_nameCombo->setCurrentIndex(idx);
        }
    }
    // No 'else' clause here - leave selection unchanged if no assignment exists

    // Update other UI elements (buttons, labels, etc.)
    m_prevBtn->setEnabled(m_currentIndex > 0);
    m_nextBtn->setEnabled(m_currentIndex < m_cardFiles.size() - 1);

    // Update button states
    m_unassignBtn->setEnabled(m_assignments.contains(m_currentIndex));
    m_assignBtn->setEnabled(true);

    m_progressLabel->setText(
        QString("Card %1/%2 | Assigned: %3")
            .arg(m_currentIndex + 1)
            .arg(m_cardFiles.size())
            .arg(m_assignments.size())
        );

    // Visual feedback
    m_imageLabel->setStyleSheet(
        m_assignments.contains(m_currentIndex)
            ? "border: 3px solid #4CAF50;"  // Green for assigned
            : "border: 2px solid gray;"     // Gray for unassigned
        );
}


void TarotOrderDialog::assignTemporarily()
{
    QString selectedId = m_nameCombo->currentData().toString();

    // Immediate feedback if card already assigned
    for (const auto& [position, cardId] : m_assignments.asKeyValueRange()) {
        if (cardId == selectedId && position != m_currentIndex) {
            QMessageBox::warning(this, "Duplicate Card",
                                 QString("\"%1\" is already assigned to position %2\n"
                                         "Each card must be unique in the deck.")
                                     .arg(m_nameCombo->currentText())
                                     .arg(position + 1));
            return;
        }
    }

    m_assignments[m_currentIndex] = selectedId;
    updateUI();
}
void TarotOrderDialog::moveNext() {
    if (m_currentIndex < m_cardFiles.size() - 1) {
        m_currentIndex++;
        loadCurrentCard();
    }
}

void TarotOrderDialog::movePrev() {
    if (m_currentIndex > 0) {
        m_currentIndex--;
        loadCurrentCard();
    }
}


void TarotOrderDialog::confirmAll()
{
    // First check for 78 cards
    if (m_assignments.size() != 78) {
        QMessageBox::warning(this, "Incomplete",
                             "Please assign all 78 cards before confirming.");
        return;
    }

    // Then verify all cards are unique
    QSet<QString> uniqueCards;
    for (const QString& cardId : m_assignments) {
        if (uniqueCards.contains(cardId)) {
            QMessageBox::critical(this, "Duplicates Found",
                                  "Some cards are assigned to multiple positions!\n"
                                  "Please fix duplicates before saving.");
            return;
        }
        uniqueCards.insert(cardId);
    }

    // Final confirmation
    if (QMessageBox::question(this, "Confirm",
                              "Save all 78 card assignments?",
                              QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes)
    {
        finalizeRenaming();
        accept();
    }
}



void TarotOrderDialog::cancelProcess() {
    if (!m_confirmed) {
        restoreOriginals();
    }
    reject();
}




void TarotOrderDialog::backupOriginals() {
    QDir dir(m_deckPath);
    for (const QString& file : dir.entryList({"*.jpg", "*.png"})) {
        QString backupPath = dir.filePath(file + ".bak");
        if (QFile::exists(backupPath)) {  // Prevent duplicate backups
            QFile::remove(backupPath);
        }
        if (QFile::copy(dir.filePath(file), backupPath)) {  // Verify copy succeeded
            m_cardFiles << backupPath;
        }
    }
}

void TarotOrderDialog::finalizeRenaming() {
    QDir dir(m_deckPath);

    // 1. Create reverse lookup: cardName -> tarotID (e.g. "The Magician" -> "01")
    QMap<QString, QString> nameToId;
    for (auto it = tarotNames.constBegin(); it != tarotNames.constEnd(); ++it) {
        nameToId[it.value()] = it.key(); // Reverse the tarotNames map
    }

    // 2. Rename using tarotIDs
    for (int i = 0; i < m_cardFiles.size(); ++i) {
        QString cardName = m_nameCombo->itemText(m_nameCombo->findData(m_assignments[i]));
        QString tarotId = nameToId.value(cardName, "99"); // "99" as fallback (should never happen)
        //QString newName = tarotId + ".png"; // "01.jpg", "00.jpg", etc.
        QString ext = detectDominantImageFormat();
        QString newName = tarotId + "." + ext;
        QString destPath = dir.filePath(newName);

        if (QFile::exists(destPath)) {
            QFile::remove(destPath);
        }
        QFile::rename(m_cardFiles[i], destPath);
    }
}


void TarotOrderDialog::restoreOriginals() {
    for (const QString& file : m_cardFiles) {
        QString originalName = file.left(file.length() - 4);
        if (QFile::exists(originalName)) {  // Don't overwrite existing originals
            QFile::remove(originalName);
        }
        QFile::rename(file, originalName);
    }
}

void TarotOrderDialog::unassignCurrent()
{
    if (m_assignments.contains(m_currentIndex)) {
        m_assignments.remove(m_currentIndex);
        updateUI();
    }
}

QString TarotOrderDialog::detectDominantImageFormat() const {
    int jpgCount = 0, pngCount = 0;
    int checked = 0;

    for (const QString& filePath : m_cardFiles) {
        if (checked >= 7) break;

        QString fileName = QFileInfo(filePath).fileName().toLower();
        if (fileName.contains("jpg") || fileName.contains("jpeg")) {
            jpgCount++;
        } else if (fileName.contains("png")) {
            pngCount++;
        }

        checked++;
    }

    if (jpgCount == 0 && pngCount == 0) {
        qWarning() << "No recognizable image formats in first 7 files.";
        return "png";  // Safe fallback
    }

    return (jpgCount > pngCount) ? "jpg" : "png";
}

