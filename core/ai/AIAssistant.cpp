#include "AIAssistant.h"
#include "../model/Concept.h"
#include "../common/QLinkException.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QEventLoop>
#include <QTimer>
#include <QUrl>
#include <QDebug>
#include <QRegularExpression>
#include <QFile>
#include <QTextStream>
#include <QIODevice>
#include <unordered_map>
#include <iostream>

namespace qlink {

class AIAssistant::Impl {
public:
    QNetworkAccessManager* networkManager;
    QString apiKey;
    QString apiEndpoint;
    bool serviceAvailable;
    int timeoutMs;
    
    // Caching system
    std::unordered_map<std::string, std::string> explanationCache;
    std::unordered_map<std::string, std::string> descriptionCache;
    std::unordered_map<std::string, std::vector<std::string>> suggestionCache;
    
    Impl() : networkManager(new QNetworkAccessManager()), 
             apiEndpoint("https://api.cohere.ai/v1/chat"),
             serviceAvailable(false),
             timeoutMs(10000) {
        loadFromEnvFile();
        serviceAvailable = !apiKey.isEmpty();
        qDebug() << "Cohere API key:" << (apiKey.isEmpty() ? "NOT FOUND" : "LOADED");
    }
    
    ~Impl() {
        delete networkManager;
    }
    
    void loadFromEnvFile() {
        QFile envFile(".env");
        if (envFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&envFile);
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (line.startsWith("COHERE_API_KEY=")) {
                    QString value = line.mid(15); // Skip "COHERE_API_KEY="
                    apiKey = value;
                    break;
                }
            }
            envFile.close();
        }
    }
    
    QString makeApiRequest(const QString& prompt, const QString& systemMessage = "") {
        if (!serviceAvailable) {
            return "";
        }
        
        QJsonObject requestBody;
        requestBody["model"] = "command-r-08-2024"; // Current active Cohere model
        requestBody["max_tokens"] = 150;
        requestBody["temperature"] = 0.7;
        
        // Combine system message and prompt for Cohere
        QString fullMessage = prompt;
        if (!systemMessage.isEmpty()) {
            fullMessage = systemMessage + "\n\n" + prompt;
        }
        requestBody["message"] = fullMessage;
        
        QJsonDocument doc(requestBody);
        QByteArray data = doc.toJson();
        
        QNetworkRequest request((QUrl(apiEndpoint)));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());
        
        QNetworkReply* reply = networkManager->post(request, data);
        
        // Wait for response with timeout
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        
        timer.start(timeoutMs);
        loop.exec();
        
        QString result;
        if (timer.isActive()) {
            timer.stop();
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray responseData = reply->readAll();
                QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);
                QJsonObject responseObj = responseDoc.object();
                
                // Cohere returns the response in a "text" field
                if (responseObj.contains("text")) {
                    result = responseObj["text"].toString().trimmed();
                }
            } else {
                qWarning() << "Cohere API request failed:" << reply->errorString();
                QByteArray errorData = reply->readAll();
                qWarning() << "Error response:" << errorData;
            }
        } else {
            qWarning() << "Cohere API request timed out";
            reply->abort();
        }
        
        reply->deleteLater();
        return result;
    }
};

AIAssistant::AIAssistant() : pImpl(std::make_unique<Impl>()) {
}

AIAssistant::~AIAssistant() = default;

std::string AIAssistant::explainConnection(const Concept& concept1, const Concept& concept2) {
    // Create cache key
    std::string cacheKey = concept1.getName() + "|" + concept2.getName();
    
    // Check cache first
    auto it = pImpl->explanationCache.find(cacheKey);
    if (it != pImpl->explanationCache.end()) {
        return it->second;
    }
    
    if (!isServiceAvailable()) {
        std::string fallback = "These concepts may be related based on their shared connections. " +
                              concept1.getName() + " and " + concept2.getName() + 
                              " could have conceptual similarities or dependencies.";
        pImpl->explanationCache[cacheKey] = fallback;
        return fallback;
    }
    
    QString systemMessage = "You are an expert at explaining relationships between concepts in knowledge graphs. "
                           "Provide concise, insightful explanations about how two concepts might be related.";
    
    QString prompt = QString("Explain the potential relationship between these two concepts:\n"
                           "1. %1: %2\n"
                           "2. %3: %4\n\n"
                           "Provide a brief explanation of how they might be connected or related.")
                    .arg(QString::fromStdString(concept1.getName()))
                    .arg(QString::fromStdString(concept1.getDescription()))
                    .arg(QString::fromStdString(concept2.getName()))
                    .arg(QString::fromStdString(concept2.getDescription()));
    
    QString response = pImpl->makeApiRequest(prompt, systemMessage);
    std::string result = response.toStdString();
    
    if (result.empty()) {
        result = "Unable to generate explanation at this time. These concepts may share "
                "common themes, dependencies, or be part of the same domain.";
    }
    
    // Cache the result
    pImpl->explanationCache[cacheKey] = result;
    return result;
}

std::string AIAssistant::generateConceptDescription(const std::string& conceptName) {
    // Check cache first
    auto it = pImpl->descriptionCache.find(conceptName);
    if (it != pImpl->descriptionCache.end()) {
        return it->second;
    }
    
    if (!isServiceAvailable()) {
        std::string fallback = "A concept in your mental model: " + conceptName;
        pImpl->descriptionCache[conceptName] = fallback;
        return fallback;
    }
    
    QString systemMessage = "You are an expert at providing clear, concise descriptions of concepts. "
                           "Provide educational and informative descriptions suitable for knowledge management.";
    
    QString prompt = QString("Provide a brief, informative description of the concept: %1\n\n"
                           "Keep it concise (1-2 sentences) and focus on the key aspects that would be "
                           "useful in a knowledge graph or mental model.")
                    .arg(QString::fromStdString(conceptName));
    
    QString response = pImpl->makeApiRequest(prompt, systemMessage);
    std::string result = response.toStdString();
    
    if (result.empty()) {
        result = "A concept representing " + conceptName + " in your knowledge model.";
    }
    
    // Cache the result
    pImpl->descriptionCache[conceptName] = result;
    return result;
}

std::vector<std::string> AIAssistant::suggestRelatedConcepts(const Concept& concept) {
    std::string cacheKey = concept.getName();
    
    // Check cache first
    auto it = pImpl->suggestionCache.find(cacheKey);
    if (it != pImpl->suggestionCache.end()) {
        return it->second;
    }
    
    if (!isServiceAvailable()) {
        // Provide some basic fallback suggestions based on tags
        std::vector<std::string> fallback;
        const auto& tags = concept.getTags();
        for (const auto& tag : tags) {
            if (fallback.size() < 3) {
                fallback.push_back("Related to " + tag);
            }
        }
        if (fallback.empty()) {
            fallback.push_back("Similar concepts");
            fallback.push_back("Related topics");
        }
        pImpl->suggestionCache[cacheKey] = fallback;
        return fallback;
    }
    
    QString systemMessage = "You are an expert at suggesting related concepts for knowledge graphs. "
                           "Provide a list of 3-5 related concept names that would be relevant to add to a mental model.";
    
    QString prompt = QString("Given this concept:\n"
                           "Name: %1\n"
                           "Description: %2\n"
                           "Tags: %3\n\n"
                           "Suggest 3-5 related concepts that would be valuable to include in a knowledge graph. "
                           "Provide only the concept names, one per line, without explanations.")
                    .arg(QString::fromStdString(concept.getName()))
                    .arg(QString::fromStdString(concept.getDescription()))
                    .arg(QString::fromStdString([&]() {
                        std::string tagStr;
                        for (const auto& tag : concept.getTags()) {
                            if (!tagStr.empty()) tagStr += ", ";
                            tagStr += tag;
                        }
                        return tagStr;
                    }()));
    
    QString response = pImpl->makeApiRequest(prompt, systemMessage);
    
    std::vector<std::string> suggestions;
    if (!response.isEmpty()) {
        QStringList lines = response.split('\n', Qt::SkipEmptyParts);
        for (const QString& line : lines) {
            QString trimmed = line.trimmed();
            if (!trimmed.isEmpty() && suggestions.size() < 5) {
                // Remove any numbering or bullet points
                trimmed = trimmed.replace(QRegularExpression("^[0-9]+\\.\\s*"), "");
                trimmed = trimmed.replace(QRegularExpression("^[-*]\\s*"), "");
                suggestions.push_back(trimmed.toStdString());
            }
        }
    }
    
    if (suggestions.empty()) {
        suggestions.push_back("Related concept 1");
        suggestions.push_back("Related concept 2");
        suggestions.push_back("Related concept 3");
    }
    
    // Cache the result
    pImpl->suggestionCache[cacheKey] = suggestions;
    return suggestions;
}

bool AIAssistant::isServiceAvailable() const {
    return pImpl->serviceAvailable;
}

void AIAssistant::setApiKey(const QString& apiKey) {
    pImpl->apiKey = apiKey;
    pImpl->serviceAvailable = !apiKey.isEmpty();
}

void AIAssistant::setApiEndpoint(const QString& endpoint) {
    pImpl->apiEndpoint = endpoint;
}

void AIAssistant::setTimeout(int milliseconds) {
    pImpl->timeoutMs = milliseconds;
}

void AIAssistant::clearCache() {
    pImpl->explanationCache.clear();
    pImpl->descriptionCache.clear();
    pImpl->suggestionCache.clear();
}

size_t AIAssistant::getCacheSize() const {
    return pImpl->explanationCache.size() + 
           pImpl->descriptionCache.size() + 
           pImpl->suggestionCache.size();
}

} // namespace qlink