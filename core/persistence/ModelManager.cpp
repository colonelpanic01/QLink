#include "ModelManager.h"
#include "../common/QLinkException.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

namespace qlink {

ModelManager::ModelManager(QObject *parent)
    : QObject(parent) {
    // Initialize default save directory
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    defaultSaveDirectory = QDir(appDataPath).absoluteFilePath("models");
    
    // Ensure directory exists
    QDir().mkpath(defaultSaveDirectory);
}

ModelManager::~ModelManager() = default;

bool ModelManager::saveModel(const MentalModel& model, const QString& filePath) {
    try {
        // Ensure file has .json extension
        QString actualFilePath = filePath;
        if (!actualFilePath.endsWith(".json", Qt::CaseInsensitive)) {
            actualFilePath += ".json";
        }
        
        QJsonObject jsonModel = serializeModel(model);
        QJsonDocument doc(jsonModel);
        QByteArray data = doc.toJson(QJsonDocument::Indented); // Use indented format for readability
        
        QFile file(actualFilePath);
        if (!file.open(QIODevice::WriteOnly)) {
            emit errorOccurred(QString("Failed to open file for writing: %1").arg(actualFilePath));
            return false;
        }
        
        qint64 bytesWritten = file.write(data);
        if (bytesWritten == -1) {
            emit errorOccurred(QString("Failed to write data to file: %1").arg(actualFilePath));
            return false;
        }
        
        file.close();
        addToRecentFiles(actualFilePath); // Automatically add to recent files
        emit modelSaved(actualFilePath);
        return true;
    } catch (const std::exception& e) {
        emit errorOccurred(QString("Error saving model: %1").arg(e.what()));
        return false;
    }
}

std::unique_ptr<MentalModel> ModelManager::loadModel(const QString& filePath) {
    try {
        // Validate file extension
        if (!filePath.endsWith(".json", Qt::CaseInsensitive)) {
            emit errorOccurred(QString("Only JSON files (.json) are supported for loading: %1").arg(filePath));
            return nullptr;
        }
        
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            emit errorOccurred(QString("Failed to open file for reading: %1").arg(filePath));
            return nullptr;
        }
        
        QByteArray data = file.readAll();
        file.close();
        
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            emit errorOccurred(QString("JSON parse error: %1").arg(parseError.errorString()));
            return nullptr;
        }
        
        if (!doc.isObject()) {
            emit errorOccurred("Invalid JSON format: root element is not an object");
            return nullptr;
        }
        
        QJsonObject rootObject = doc.object();
        
        // Check if there's a "model" wrapper object
        QJsonObject modelObject;
        if (rootObject.contains("model") && rootObject["model"].isObject()) {
            modelObject = rootObject["model"].toObject();
        } else {
            // Assume the root object is the model itself
            modelObject = rootObject;
        }
        
        auto model = deserializeModel(modelObject);
        if (model) {
            addToRecentFiles(filePath); // Automatically add to recent files
            emit modelLoaded(filePath);
        }
        return model;
    } catch (const std::exception& e) {
        emit errorOccurred(QString("Error loading model: %1").arg(e.what()));
        return nullptr;
    }
}

bool ModelManager::exportModel(const MentalModel& model, const QString& filePath, ExportFormat format) {
    try {
        // Only JSON export is supported
        if (format != ExportFormat::JSON) {
            emit errorOccurred("Only JSON export format is supported");
            return false;
        }
        return exportToJSON(model, filePath);
    } catch (const std::exception& e) {
        emit errorOccurred(QString("Error exporting model: %1").arg(e.what()));
        return false;
    }
}

QStringList ModelManager::getRecentFiles() const {
    return recentFiles;
}

void ModelManager::addToRecentFiles(const QString& filePath) {
    recentFiles.removeAll(filePath); // Remove if already exists
    recentFiles.prepend(filePath); // Add to front
    // Keep only the last 10 files
    while (recentFiles.size() > 10) {
        recentFiles.removeLast();
    }
    emit recentFilesChanged(recentFiles);
}

QString ModelManager::getDefaultSaveDirectory() const {
    return defaultSaveDirectory;
}

void ModelManager::setDefaultSaveDirectory(const QString& directory) {
    defaultSaveDirectory = directory;
    QDir().mkpath(directory); // Ensure it exists
}

QJsonObject ModelManager::serializeModel(const MentalModel& model) {
    QJsonObject jsonModel;
    // Basic model information
    jsonModel["name"] = QString::fromStdString(model.getModelName());
    jsonModel["version"] = "1.0";
    jsonModel["created"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    // Serialize concepts
    QJsonArray conceptsArray;
    for (const auto& concept : model.getConcepts()) {
        conceptsArray.append(serializeConcept(*concept));
    }
    jsonModel["concepts"] = conceptsArray;
    
    // Serialize relationships
    QJsonArray relationshipsArray;
    for (const auto& relationship : model.getRelationships()) {
        relationshipsArray.append(serializeRelationship(*relationship));
    }
    jsonModel["relationships"] = relationshipsArray;
    
    // Add statistics for verification
    auto stats = model.getStatistics();
    QJsonObject statsObject;
    statsObject["conceptCount"] = static_cast<int>(stats.conceptCount);
    statsObject["relationshipCount"] = static_cast<int>(stats.relationshipCount);
    statsObject["orphanedConceptCount"] = static_cast<int>(stats.orphanedConceptCount);
    statsObject["averageConnections"] = stats.averageConnections;
    statsObject["maxConnections"] = static_cast<int>(stats.maxConnections);
    statsObject["minConnections"] = static_cast<int>(stats.minConnections);
    jsonModel["statistics"] = statsObject;
    
    return jsonModel;
}

std::unique_ptr<MentalModel> ModelManager::deserializeModel(const QJsonObject& jsonModel) {
    // Extract model name
    QString modelName = jsonModel["name"].toString("Untitled Model");
    auto model = std::make_unique<MentalModel>(modelName.toStdString());
    
    // Deserialize concepts first
    QJsonArray conceptsArray = jsonModel["concepts"].toArray();
    for (const auto& conceptValue : conceptsArray) {
        if (conceptValue.isObject()) {
            auto concept = deserializeConcept(conceptValue.toObject());
            if (concept) {
                model->addConcept(std::move(concept));
            }
        }
    }
    
    // Then deserialize relationships
    QJsonArray relationshipsArray = jsonModel["relationships"].toArray();
    for (const auto& relationshipValue : relationshipsArray) {
        if (relationshipValue.isObject()) {
            auto relationship = deserializeRelationship(relationshipValue.toObject());
            if (relationship) {
                // Verify that the concepts exist
                if (model->getConcept(relationship->getSourceConceptId()) &&
                    model->getConcept(relationship->getTargetConceptId())) {
                    model->addRelationship(std::move(relationship));
                } else {
                    qWarning() << "Skipping relationship with invalid concept IDs";
                }
            }
        }
    }
    
    return model;
}

QJsonObject ModelManager::serializeConcept(const Concept& concept) {
    QJsonObject jsonConcept;
    jsonConcept["id"] = QString::fromStdString(concept.getId());
    jsonConcept["name"] = QString::fromStdString(concept.getName());
    jsonConcept["description"] = QString::fromStdString(concept.getDescription());
    
    // Serialize tags
    QJsonArray tagsArray;
    for (const auto& tag : concept.getTags()) {
        tagsArray.append(QString::fromStdString(tag));
    }
    jsonConcept["tags"] = tagsArray;
    
    // Serialize position
    auto position = concept.getPosition();
    QJsonObject positionObject;
    positionObject["x"] = position.x;
    positionObject["y"] = position.y;
    jsonConcept["position"] = positionObject;
    
    // Add timestamps
    jsonConcept["created"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    jsonConcept["modified"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    return jsonConcept;
}

std::unique_ptr<Concept> ModelManager::deserializeConcept(const QJsonObject& jsonConcept) {
    QString id = jsonConcept["id"].toString();
    QString name = jsonConcept["name"].toString();
    QString description = jsonConcept["description"].toString();
    
    if (id.isEmpty() || name.isEmpty()) {
        qWarning() << "Invalid concept data: missing id or name";
        return nullptr;
    }
    
    auto concept = std::make_unique<Concept>(id.toStdString(), name.toStdString(), description.toStdString());
    
    // Deserialize tags
    QJsonArray tagsArray = jsonConcept["tags"].toArray();
    for (const auto& tagValue : tagsArray) {
        concept->addTag(tagValue.toString().toStdString());
    }
    
    // Deserialize position
    QJsonObject positionObject = jsonConcept["position"].toObject();
    Position position;
    position.x = positionObject["x"].toDouble();
    position.y = positionObject["y"].toDouble();
    concept->setPosition(position);
    
    return concept;
}

QJsonObject ModelManager::serializeRelationship(const Relationship& relationship) {
    QJsonObject jsonRelationship;
    jsonRelationship["id"] = QString::fromStdString(relationship.getId());
    jsonRelationship["sourceConceptId"] = QString::fromStdString(relationship.getSourceConceptId());
    jsonRelationship["targetConceptId"] = QString::fromStdString(relationship.getTargetConceptId());
    jsonRelationship["type"] = QString::fromStdString(relationship.getType());
    jsonRelationship["directed"] = relationship.getIsDirected();
    jsonRelationship["weight"] = relationship.getWeight();
    
    // Add timestamps
    jsonRelationship["created"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    jsonRelationship["modified"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    return jsonRelationship;
}

std::unique_ptr<Relationship> ModelManager::deserializeRelationship(const QJsonObject& jsonRelationship) {
    QString sourceId = jsonRelationship["sourceConceptId"].toString();
    QString targetId = jsonRelationship["targetConceptId"].toString();
    QString type = jsonRelationship["type"].toString();
    bool directed = jsonRelationship["directed"].toBool();
    double weight = jsonRelationship["weight"].toDouble();
    
    if (sourceId.isEmpty() || targetId.isEmpty() || type.isEmpty()) {
        qWarning() << "Invalid relationship data: missing required fields";
        return nullptr;
    }
    
    auto relationship = std::make_unique<Relationship>(
        sourceId.toStdString(),
        targetId.toStdString(),
        type.toStdString(),
        directed,
        weight
    );
    
    return relationship;
}

bool ModelManager::exportToJSON(const MentalModel& model, const QString& filePath) {
    // Ensure file has .json extension
    QString actualFilePath = filePath;
    if (!actualFilePath.endsWith(".json", Qt::CaseInsensitive)) {
        actualFilePath += ".json";
    }
    
    QJsonObject jsonModel = serializeModel(model);
    // Add export metadata
    jsonModel["exportFormat"] = "JSON";
    jsonModel["exportedAt"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    QJsonDocument doc(jsonModel);
    QByteArray data = doc.toJson(QJsonDocument::Indented);
    
    QFile file(actualFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit errorOccurred(QString("Failed to open file for JSON export: %1").arg(actualFilePath));
        return false;
    }
    
    file.write(data);
    file.close();
    
    emit modelExported(actualFilePath, ExportFormat::JSON);
    return true;
}

} // namespace qlink