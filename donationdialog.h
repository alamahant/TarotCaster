#ifndef DONATIONDIALOG_H
#define DONATIONDIALOG_H

#include <QDialog>

class QVBoxLayout;
class QLabel;

class DonationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DonationDialog(QWidget *parent = nullptr);

private slots:
    void onCopyClicked();

private:
    void setupUI();
    void setupHeader();
    void setupDonationPlatforms();
    void setupFooter();
    QWidget* createDonationPlatform(const QString &title, const QString &url, const QString &displayUrl);
    QString getHeaderContent() const;
    void copyToClipboard(const QString &url);

    QVBoxLayout *m_mainLayout;
};

#endif // DONATIONDIALOG_H
