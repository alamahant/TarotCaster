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

    // Create the JSON request for Mistral AI
    QJsonDocument createRequestJson(const QString& prompt);
    QString query;
public:
    QString getApiKey() const { return m_apiKey; }
    void setQuery(const QString &newQuery);

private:
    QString m_apiKey;
    QString m_apiEndpoint;
    QString m_model;
    int m_maxTokens;
    double m_temperature;
    QString m_lastError;
public slots:
        bool loadActiveModel();
};

