#include "donationdialog.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QApplication>
#include <QClipboard>

DonationDialog::DonationDialog(QWidget *parent)
    : QDialog(parent)
    , m_mainLayout(nullptr)
{
    setupUI();
}

void DonationDialog::setupUI()
{
    setWindowTitle(tr("Support %1").arg(QApplication::applicationName()));
    setMinimumSize(500, 500);
    setMaximumSize(800, 600);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(15);

    setupHeader();
    setupDonationPlatforms();
    setupFooter();
}

void DonationDialog::setupHeader()
{
    QLabel *headerLabel = new QLabel(getHeaderContent());
    headerLabel->setWordWrap(true);
    headerLabel->setTextFormat(Qt::RichText);
    headerLabel->setOpenExternalLinks(true);
    m_mainLayout->addWidget(headerLabel);
}

void DonationDialog::setupDonationPlatforms()
{
    createDonationPlatform("☕ Buy Me a Coffee",
                          "https://buymeacoffee.com/Alamahant",
                          "buymeacoffee.com/Alamahant");

    createDonationPlatform("❤️ Ko-fi",
                          "https://ko-fi.com/alamahant",
                          "ko-fi.com/alamahant");

    createDonationPlatform("💰 PayPal",
                          "https://paypal.me/Alamahant",
                          "paypal.me/Alamahant");
}

void DonationDialog::setupFooter()
{
    // Support benefits section
    QLabel *benefitsLabel = new QLabel(
        "<div style='background-color: #f8f9fa; padding: 15px; border-radius: 8px;'>"
        "<p style='margin: 0; font-size: 14px; color: #2c3e50; font-weight: bold; line-height: 1.5;'>"
        "✨ Your support enables:<br>"
        "• New features and improvements<br>"
        "• Bug fixes and maintenance<br>"
        "• Future updates and compatibility"
        "</p></div>"
    );
    benefitsLabel->setTextFormat(Qt::RichText);
    benefitsLabel->setWordWrap(true);
    m_mainLayout->addWidget(benefitsLabel);

    m_mainLayout->addStretch();

    // Close button
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    m_mainLayout->addWidget(buttonBox);
}

QWidget* DonationDialog::createDonationPlatform(const QString &title, const QString &url, const QString &displayUrl)
{
    QWidget *platformWidget = new QWidget();
    QHBoxLayout *platformLayout = new QHBoxLayout(platformWidget);
    platformLayout->setContentsMargins(20, 5, 20, 5);

    // Platform link
    QLabel *linkLabel = new QLabel(
        QString("<b>%1:</b> <a href=\"%2\" style=\"color: #3498db;\">%3</a>")
            .arg(title, url, displayUrl)
    );
    linkLabel->setTextFormat(Qt::RichText);
    linkLabel->setOpenExternalLinks(true);
    platformLayout->addWidget(linkLabel, 1);

    // Copy button
    QPushButton *copyButton = new QPushButton(tr("Copy"));
    copyButton->setFixedSize(60, 25);
    copyButton->setProperty("donationUrl", url);
    connect(copyButton, &QPushButton::clicked, this, &DonationDialog::onCopyClicked);
    platformLayout->addWidget(copyButton);

    m_mainLayout->addWidget(platformWidget);
    return platformWidget;
}

void DonationDialog::onCopyClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (button) {
        QString url = button->property("donationUrl").toString();
        copyToClipboard(url);
        QMessageBox::information(this, tr("Success"), tr("URL copied to clipboard!"));
    }
}

void DonationDialog::copyToClipboard(const QString &url)
{
    QApplication::clipboard()->setText(url);
}

QString DonationDialog::getHeaderContent() const
{
    return QString(
        "<div style='text-align: center; color: #2c3e50;'>"
        "<h2 style='color: #2c3e50; margin-bottom: 15px;'>❤️ Support %1</h2>"
        "<p style='font-size: 14px; line-height: 1.5; margin-bottom: 15px;'>"
        "If you find <strong>%1</strong> useful and you enjoy using it, "
        "please consider supporting its development. Your donation "
        "helps maintain and improve this application.</p>"
        "<p style='font-size: 14px; line-height: 1.5;'>"
        "Every contribution, no matter how small, makes a difference "
        "and is greatly appreciated!</p>"
        "</div>"
    ).arg(QApplication::applicationName());
}
