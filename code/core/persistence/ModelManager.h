#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QDateTime>
#include <memory>
#include "../model/MentalModel.h"

namespace qlink {

/**
 * Export format, JSON only currently may add more in future or next deliverable
 */
enum class ExportFormat {
    JSON
};

/**
 * Manages the persistence operations for our mental models
 */
class ModelManager : public QObject {
    Q_OBJECT

public:
    explicit ModelManager(QObject *parent = nullptr);
    ~ModelManager();

    // Core persistence operations
    bool saveModel(const MentalModel& model, const QString& filePath);
    std::unique_ptr<MentalModel> loadModel(const QString& filePath);
    bool exportModel(const MentalModel& model, const QString& filePath, ExportFormat format);

    // File management
    QStringList getRecentFiles() const;
    void addToRecentFiles(const QString& filePath);
    QString getDefaultSaveDirectory() const;
    void setDefaultSaveDirectory(const QString& directory);

signals:
    void modelSaved(const QString& filePath);
    void modelLoaded(const QString& filePath);
    void modelExported(const QString& filePath, ExportFormat format);
    void errorOccurred(const QString& error);
    void recentFilesChanged(const QStringList& recentFiles);

private:
    // Serialization methods
    QJsonObject serializeModel(const MentalModel& model);
    std::unique_ptr<MentalModel> deserializeModel(const QJsonObject& jsonModel);
    QJsonObject serializeConcept(const Concept& concept);
    std::unique_ptr<Concept> deserializeConcept(const QJsonObject& jsonConcept);
    QJsonObject serializeRelationship(const Relationship& relationship);
    std::unique_ptr<Relationship> deserializeRelationship(const QJsonObject& jsonRelationship);

    // Export format implementation
    bool exportToJSON(const MentalModel& model, const QString& filePath);

    // Member variables
    QStringList recentFiles;
    QString defaultSaveDirectory;
};

} // namespace qlink