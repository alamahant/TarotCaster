#include "customspreaddesigner.h"
#include "tarotscene.h"
#include "tarotcarditem.h"

#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QFormLayout>
//#include <QGraphicsView>
CustomSpreadDesigner::CustomSpreadDesigner(TarotScene* scene, QWidget* parent)
    : QDialog(parent),
    m_scene(scene),
    m_currentPositionNumber(1),
    m_selectedPositionIndex(-1),
    m_spreadNameEdit(nullptr),
    m_spreadDescriptionEdit(nullptr),
    m_positionNumberLabel(nullptr),
    m_positionNameEdit(nullptr),
    m_positionSignificanceEdit(nullptr),
    m_addCardButton(nullptr),
    m_saveSpreadButton(nullptr),
    m_positionsList(nullptr)
{
    setWindowTitle("Custom Spread Designer");
    setMinimumSize(250, 300);

    // Don't modify window flags here - use default QDialog flags

    setupUI();


}

CustomSpreadDesigner::~CustomSpreadDesigner()
{
    // Clean up any temporary cards in the scene if dialog is closed without saving
    if (m_scene) {
        for (auto item : m_scene->items()) {
            if (item && item->data(0).isValid() && item->data(0).toString() == "spreadDesignerCard") {
                m_scene->removeItem(item);
                delete item;
            }
        }
    }
}

void CustomSpreadDesigner::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Spread Information Group
    QGroupBox* spreadInfoGroup = new QGroupBox("Spread Information", this);
    QFormLayout* spreadInfoLayout = new QFormLayout(spreadInfoGroup);
    m_spreadNameEdit = new QLineEdit(this);
    spreadInfoLayout->addRow("Spread Name:", m_spreadNameEdit);
    m_spreadDescriptionEdit = new QTextEdit(this);
    m_spreadDescriptionEdit->setMaximumHeight(80);
    spreadInfoLayout->addRow("Spread Description:", m_spreadDescriptionEdit);
    mainLayout->addWidget(spreadInfoGroup);

    // Position Details Group
    QGroupBox* positionGroup = new QGroupBox("Position Details", this);
    QFormLayout* positionLayout = new QFormLayout(positionGroup);
    m_positionNumberLabel = new QLabel("1", this);
    positionLayout->addRow("Position Number:", m_positionNumberLabel);
    m_positionNameEdit = new QLineEdit(this);
    positionLayout->addRow("Position Name:", m_positionNameEdit);
    m_positionSignificanceEdit = new QTextEdit(this);
    positionLayout->addRow("Position Significance:", m_positionSignificanceEdit);
    mainLayout->addWidget(positionGroup);

    // Position List Group
    QGroupBox* positionsListGroup = new QGroupBox("Defined Positions", this);
    QVBoxLayout* positionsListLayout = new QVBoxLayout(positionsListGroup);
    m_positionsList = new QListWidget(this);
    positionsListLayout->addWidget(m_positionsList);
    mainLayout->addWidget(positionsListGroup);

    // Buttons
    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    m_addCardButton = new QPushButton("Add Card", this);
    buttonsLayout->addWidget(m_addCardButton);
    //managespreads button
    m_manageSpreadsButton = new QPushButton("Manage Spreads", this);
    buttonsLayout->addWidget(m_manageSpreadsButton);

    buttonsLayout->addStretch();

    m_saveSpreadButton = new QPushButton("Save Spread", this);
    m_saveSpreadButton->setEnabled(false);
    buttonsLayout->addWidget(m_saveSpreadButton);

    QPushButton* cancelButton = new QPushButton("Cancel", this);
    buttonsLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonsLayout);

    // Connect signals
    connect(m_addCardButton, &QPushButton::clicked, this, &CustomSpreadDesigner::addCard);
    connect(m_manageSpreadsButton, &QPushButton::clicked, this, &CustomSpreadDesigner::manageCustomSpreads);

    connect(m_saveSpreadButton, &QPushButton::clicked, this, &CustomSpreadDesigner::saveSpread);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_positionsList, &QListWidget::currentRowChanged, this, &CustomSpreadDesigner::positionSelected);

    // Connect text fields to update positions when edited
    connect(m_positionNameEdit, &QLineEdit::textChanged, [this]() {
        if (m_selectedPositionIndex >= 0 && m_selectedPositionIndex < m_positions.size()) {
            m_positions[m_selectedPositionIndex].name = m_positionNameEdit->text();
            updatePositionList();
        }
    });

    connect(m_positionSignificanceEdit, &QTextEdit::textChanged, [this]() {
        if (m_selectedPositionIndex >= 0 && m_selectedPositionIndex < m_positions.size()) {
            m_positions[m_selectedPositionIndex].significance = m_positionSignificanceEdit->toPlainText();
        }
    });

    // Initial state
    m_positionNameEdit->setEnabled(false);
    m_positionSignificanceEdit->setEnabled(false);

    QString focusStyle = "QLineEdit:focus, QTextEdit:focus, QListWidget:focus { border: 2px solid #0078d7; }";
    this->setStyleSheet(focusStyle);
}



void CustomSpreadDesigner::addCard()
{
    try {
        if (!m_scene) {
            QMessageBox::warning(this, "Error", "Scene is not available");
            return;
        }

        // Get the card back pixmap
        QPixmap cardBackPixmap = m_scene->getCardLoader().getCardBack();
        if (cardBackPixmap.isNull()) {
            QMessageBox::warning(this, "Warning", "Failed to get card back image");
            return;
        }

        // Create a new card item
        TarotCardItem* card = new TarotCardItem(
            cardBackPixmap,
            cardBackPixmap,
            -1 // Special value for placeholder
            );
        card->setAcceptHoverEvents(false);

        // Set custom data to identify it
        card->setData(0, "spreadDesignerCard");
        card->setData(1, m_currentPositionNumber);

        // Position it in the center of the scene
        QPointF center = m_scene->sceneRect().center();
        card->setPos(center);

        // Add to scene
        m_scene->addItem(card);

        // Create position data
        CardPosition position;
        position.number = m_currentPositionNumber;
        position.name = QString("Position %1").arg(m_currentPositionNumber);
        position.significance = "";
        position.position = center;

        // Add to positions list
        m_positions.append(position);

        // Update UI
        updatePositionList();
        m_saveSpreadButton->setEnabled(true);

        // Enable position fields
        m_positionNameEdit->setEnabled(true);
        m_positionSignificanceEdit->setEnabled(true);

        // Increment position counter for next card
        m_currentPositionNumber++;

        // Update position number label for next card
        m_positionNumberLabel->setText(QString::number(m_currentPositionNumber));
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Exception in addCard: %1").arg(e.what()));
    } catch (...) {
        QMessageBox::critical(this, "Error", "Unknown exception in addCard");
    }
}







void CustomSpreadDesigner::saveSpread()
{

    // Update positions from scene before saving
    updatePositionsFromScene();

    if (!validateSpread()) {
        return;
    }

    // Create a custom spread object
    TarotScene::CustomSpread spread;
    spread.name = m_spreadNameEdit->text();
    spread.description = m_spreadDescriptionEdit->toPlainText();

    // Add all positions
    for (const auto& pos : m_positions) {
        TarotScene::CustomSpreadPosition position;
        position.number = pos.number;
        position.name = pos.name;
        position.significance = pos.significance;
        position.x = pos.position.x();
        position.y = pos.position.y();
        spread.positions.append(position);
    }


    // Save the spread using TarotScene
    if (m_scene && m_scene->saveCustomSpread(spread)) {
        QMessageBox::information(this, "Success",
                                 QString("Spread '%1' saved successfully with %2 positions.")
                                     .arg(spread.name)
                                     .arg(spread.positions.size()));
        // Accept the dialog
        accept();
    } else {
        QMessageBox::critical(this, "Error", "Failed to save the spread.");
    }
}




void CustomSpreadDesigner::positionSelected(int index)
{
    // Safety check
    if (index < 0 || index >= m_positions.size()) {
        m_selectedPositionIndex = -1;
        return;
    }

    // Store selected index
    m_selectedPositionIndex = index;

    // Get the selected position
    const CardPosition& position = m_positions[index];

    // Update UI with position details
    m_positionNumberLabel->setText(QString::number(position.number));
    m_positionNameEdit->setText(position.name);
    m_positionSignificanceEdit->setText(position.significance);

    // Enable editing fields
    m_positionNameEdit->setEnabled(true);
    m_positionSignificanceEdit->setEnabled(true);

    // Highlight the selected card in the scene
    highlightSelectedCard(position.number);
}



void CustomSpreadDesigner::updatePositionList()
{
    // Save the currently selected position
    int selectedRow = m_positionsList->currentRow();

    // Clear the list
    m_positionsList->clear();

    // Add all positions to the list with details
    for (const auto& position : m_positions) {
        QString itemText = QString("%1: %2")
        .arg(position.number)
            .arg(position.name);
        m_positionsList->addItem(itemText);
    }

    // Restore selection if possible
    if (selectedRow >= 0 && selectedRow < m_positionsList->count()) {
        m_positionsList->setCurrentRow(selectedRow);
    }
}







void CustomSpreadDesigner::clearPositionFields()
{
    m_positionNameEdit->clear();
    m_positionSignificanceEdit->clear();
}





bool CustomSpreadDesigner::validateCurrentPosition()
{
    if (m_positionNameEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please enter a name for this position.");
        m_positionNameEdit->setFocus();
        return false;
    }

    if (m_positionSignificanceEdit->toPlainText().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please enter the significance of this position.");
        m_positionSignificanceEdit->setFocus();
        return false;
    }

    return true;
}

bool CustomSpreadDesigner::validateSpread()
{
    if (m_spreadNameEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please enter a name for the spread.");
        m_spreadNameEdit->setFocus();
        return false;
    }

    if (m_spreadDescriptionEdit->toPlainText().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please enter a description for the spread.");
        m_spreadDescriptionEdit->setFocus();
        return false;
    }

    if (m_positions.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please add at least one position to the spread.");
        return false;
    }

    return true;
}


void CustomSpreadDesigner::updatePositionsFromScene()
{
    if (!m_scene) return;

    // Update all position coordinates from the scene
    for (int i = 0; i < m_positions.size(); i++) {
        int positionNumber = m_positions[i].number;

        // Find the card in the scene
        for (auto item : m_scene->items()) {
            if (item && item->data(0).isValid() &&
                item->data(0).toString() == "spreadDesignerCard" &&
                item->data(1).toInt() == positionNumber) {

                // Update the position
                m_positions[i].position = item->pos();
                break;
            }
        }
    }
}


void CustomSpreadDesigner::highlightSelectedCard(int positionNumber)
{
    if (!m_scene) return;

    // Reset all cards to normal
    for (auto item : m_scene->items()) {
        if (item && item->data(0).isValid() && item->data(0).toString() == "spreadDesignerCard") {
            // Reset opacity
            item->setOpacity(0.7);
        }
    }

    // Find and highlight the selected card
    for (auto item : m_scene->items()) {
        if (item && item->data(0).isValid() &&
            item->data(0).toString() == "spreadDesignerCard" &&
            item->data(1).toInt() == positionNumber) {

            // Highlight the selected card
            item->setOpacity(1.0);
            break;
        }
    }
}

void CustomSpreadDesigner::manageCustomSpreads()
{
    if (!m_scene) {
        QMessageBox::warning(this, "Error", "Scene is not available");
        return;
    }

    // Get all custom spreads
    QVector<TarotScene::CustomSpread> spreads = m_scene->getCustomSpreads();

    if (spreads.isEmpty()) {
        QMessageBox::information(this, "No Custom Spreads",
                                 "There are no custom spreads to manage.");
        return;
    }

    // Create a simple dialog
    QDialog dialog(this);
    dialog.setWindowTitle("Manage Custom Spreads");
    dialog.setMinimumSize(300, 200);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    // Create a list widget to display spreads
    QListWidget* spreadsList = new QListWidget(&dialog);
    layout->addWidget(spreadsList);

    // Add spreads to the list
    for (const auto& spread : spreads) {
        QString itemText = QString("%1 (%2 positions)")
        .arg(spread.name)
            .arg(spread.positions.size());
        QListWidgetItem* item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, spread.name); // Store the spread name for deletion
        spreadsList->addItem(item);
    }

    // Add delete button
    QPushButton* deleteButton = new QPushButton("Delete Selected Spread", &dialog);
    layout->addWidget(deleteButton);

    // Connect delete button
    connect(deleteButton, &QPushButton::clicked, [&]() {
        int currentRow = spreadsList->currentRow();
        if (currentRow >= 0 && currentRow < spreadsList->count()) {
            QString spreadName = spreadsList->currentItem()->data(Qt::UserRole).toString();

            // Confirm deletion
            QMessageBox::StandardButton confirm = QMessageBox::question(
                &dialog,
                "Confirm Deletion",
                QString("Are you sure you want to delete the spread '%1'?").arg(spreadName),
                QMessageBox::Yes | QMessageBox::No
                );

            if (confirm == QMessageBox::Yes) {
                // Delete the spread
                if (m_scene->deleteCustomSpread(spreadName)) {
                    // Remove from list
                    delete spreadsList->takeItem(currentRow);

                    // Show success message
                    QMessageBox::information(&dialog, "Success",
                                             QString("Spread '%1' has been deleted.").arg(spreadName));

                    // Close dialog if no more spreads
                    if (spreadsList->count() == 0) {
                        dialog.accept();
                    }
                } else {
                    QMessageBox::critical(&dialog, "Error",
                                          QString("Failed to delete spread '%1'.").arg(spreadName));
                }
            }
        }
    });

    // Add close button
    QPushButton* closeButton = new QPushButton("Close", &dialog);
    layout->addWidget(closeButton);
    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    // Show the dialog
    dialog.exec();
}


