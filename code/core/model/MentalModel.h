#pragma once

#include <vector>
#include <memory>
#include <string>
#include <map>
#include <QObject>
#include "Concept.h"
#include "Relationship.h"
#include "../common/DataStructures.h"

namespace qlink {

// Forward declarations
class Concept;
class Relationship;
struct ModelChangeEvent;

/**
 * Main container for concepts and relationships (Observer Pattern via Qt signals)
 */
class MentalModel : public QObject {
    Q_OBJECT

public:
    explicit MentalModel(const std::string& name = "Untitled Model", QObject* parent = nullptr);
    ~MentalModel();
    
    // Concept management
    void addConcept(std::unique_ptr<Concept> concept);
    void removeConcept(const std::string& conceptId);
    Concept* getConcept(const std::string& conceptId);
    const Concept* getConcept(const std::string& conceptId) const;
    const std::vector<std::unique_ptr<Concept>>& getConcepts() const;
    
    // Relationship management
    void addRelationship(std::unique_ptr<Relationship> relationship);
    void removeRelationship(const std::string& relationshipId);
    Relationship* getRelationship(const std::string& relationshipId);
    const Relationship* getRelationship(const std::string& relationshipId) const;
    const std::vector<std::unique_ptr<Relationship>>& getRelationships() const;
    
    // Graph operations
    std::vector<Concept*> getConnectedConcepts(const std::string& conceptId);
    std::vector<const Concept*> getConnectedConcepts(const std::string& conceptId) const;
    std::vector<Relationship*> getConceptRelationships(const std::string& conceptId);
    std::vector<const Relationship*> getConceptRelationships(const std::string& conceptId) const;
    bool areConnected(const std::string& concept1Id, const std::string& concept2Id) const;
    
    std::vector<std::string> findShortestPath(const std::string& startConceptId, 
                                             const std::string& endConceptId);
    std::vector<Concept*> getOrphanedConcepts();
    double getConceptImportance(const std::string& conceptId);
    
    // Model properties
    const std::string& getModelName() const;
    void setModelName(const std::string& name);
    size_t getConceptCount() const;
    size_t getRelationshipCount() const;
    
    // Model validation
    bool isValid() const;
    std::vector<std::string> getValidationErrors() const;
    
    // Statistics
    ModelStatistics getStatistics() const;
    
    // Utility methods
    void clear();
    bool isEmpty() const;
    
    // JSON serialization
    std::string toJson() const;
    static std::unique_ptr<MentalModel> fromJson(const std::string& json);

signals:
    void modelChanged(const ModelChangeEvent& event);
    void conceptAdded(const QString& conceptId);
    void conceptRemoved(const QString& conceptId);
    void relationshipAdded(const QString& relationshipId);
    void relationshipRemoved(const QString& relationshipId);

private:
    void notifyChange(const ModelChangeEvent& event);
    
    std::vector<std::unique_ptr<Concept>> concepts;
    std::vector<std::unique_ptr<Relationship>> relationships;
    std::string modelName;
};

} // namespace qlink