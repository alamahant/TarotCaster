#include "mistralapi.h"

#include <QNetworkRequest>
#include <QUrlQuery>
#include <QDebug>

MistralAPI::MistralAPI(QObject *parent) : QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished,
            this, &MistralAPI::handleNetworkReply);

    // Load API key from settings if available
    loadApiKey();
}

MistralAPI::~MistralAPI()
{
    // Clean up is handled by Qt's parent-child system
}

void MistralAPI::generateReading(const QString& prompt)
{
    if (apiKey.isEmpty()) {
        emit errorOccurred("API key is not set. Please set your Mistral AI API key.");
        return;
    }

    QNetworkRequest request(QUrl("https://api.mistral.ai/v1/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());

    QJsonDocument requestData = createRequestJson(prompt);

    networkManager->post(request, requestData.toJson());
}

bool MistralAPI::hasApiKey() const
{
    return !apiKey.isEmpty();
}

void MistralAPI::setApiKey(const QString& newApiKey)
{
    apiKey = newApiKey;
    saveApiKey();
}

void MistralAPI::handleNetworkReply(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
        QJsonObject jsonObject = jsonResponse.object();

        if (jsonObject.contains("choices") && jsonObject["choices"].isArray()) {
            QJsonArray choices = jsonObject["choices"].toArray();
            if (!choices.isEmpty() && choices[0].isObject()) {
                QJsonObject firstChoice = choices[0].toObject();
                if (firstChoice.contains("message") && firstChoice["message"].isObject()) {
                    QJsonObject message = firstChoice["message"].toObject();
                    if (message.contains("content") && message["content"].isString()) {
                        QString reading = message["content"].toString();
                        emit readingReady(reading);
                        reply->deleteLater();
                        return;
                    }
                }
            }
        }

        emit errorOccurred("Invalid response format from Mistral AI");
    } else {
        QString errorString = reply->errorString();
        QByteArray responseData = reply->readAll();

        // Try to parse error message from API if available
        QString detailedError = errorString;
        QJsonDocument errorJson = QJsonDocument::fromJson(responseData);
        if (!errorJson.isNull() && errorJson.isObject()) {
            QJsonObject errorObj = errorJson.object();
            if (errorObj.contains("error") && errorObj["error"].isObject()) {
                QJsonObject error = errorObj["error"].toObject();
                if (error.contains("message") && error["message"].isString()) {
                    detailedError = error["message"].toString();
                }
            }
        }

        emit errorOccurred("Network error: " + detailedError);
    }

    reply->deleteLater();
}

void MistralAPI::loadApiKey()
{
    QSettings settings;
    apiKey = settings.value("mistral/apiKey", "").toString();
}

void MistralAPI::saveApiKey()
{
    QSettings settings;
    settings.setValue("mistral/apiKey", apiKey);
}

QJsonDocument MistralAPI::createRequestJson(const QString& prompt)
{
    QJsonObject systemMessage;
    systemMessage["role"] = "system";
    systemMessage["content"] = "You are a skilled tarot reader with deep knowledge of tarot symbolism and interpretation. "
                               "Provide insightful, thoughtful readings that connect the cards to the querent's situation. "
                               "Be mystical but practical, offering both spiritual insights and actionable advice.";

    QJsonObject userMessage;
    userMessage["role"] = "user";
    userMessage["content"] = prompt;

    QJsonArray messages;
    messages.append(systemMessage);
    messages.append(userMessage);

    QJsonObject requestObject;
    requestObject["model"] = "mistral-medium";  // Use appropriate model
    requestObject["messages"] = messages;
    requestObject["temperature"] = 0.7;  // Adjust for creativity vs consistency
    requestObject["max_tokens"] = 2000;  // Adjust based on desired response length

    return QJsonDocument(requestObject);
}
