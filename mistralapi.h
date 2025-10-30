#pragma once
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>

    class MistralAPI : public QObject
{
    Q_OBJECT

public:
    explicit MistralAPI(QObject *parent = nullptr);
    ~MistralAPI();

    // Generate a tarot reading based on the provided card information
    void generateReading(const QString& prompt);

    // Check if API key exists
    bool hasApiKey() const;

    // Set API key
    void setApiKey(const QString& apiKey);

signals:
    // Signal emitted when reading is ready
    void readingReady(const QString& reading);

    // Signal emitted when an error occurs
    void errorOccurred(const QString& errorMessage);

private slots:
    // Handle network reply
    void handleNetworkReply(QNetworkReply* reply);

private:
    QNetworkAccessManager* networkManager;
    QString apiKey;

    // Load API key from settings
    void loadApiKey();

    // Save API key to settings
    void saveApiKey();

    // Create the JSON request for Mistral AI
    QJsonDocument createRequestJson(const QString& prompt);
    QString query;
public:
    QString getApiKey() const { return apiKey; }
    void setQuery(const QString &newQuery);
    };

