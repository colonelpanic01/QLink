#include "MentalModel.h"
#include <algorithm>
#include <set>
#include <queue>
#include <sstream>
#include <QString>

namespace qlink {

MentalModel::MentalModel(const std::string& name, QObject* parent)
    : QObject(parent), modelName(name) {
}

MentalModel::~MentalModel() = default;

// Concept management
void MentalModel::addConcept(std::unique_ptr<Concept> concept) {
    if (!concept) return;
    std::string conceptId = concept->getId();
    concepts.push_back(std::move(concept));
    emit conceptAdded(QString::fromStdString(conceptId));
    notifyChange(ModelChangeEvent(ChangeType::CONCEPT_ADDED, conceptId));
}

void MentalModel::removeConcept(const std::string& conceptId) {
    // First remove all relationships involving this concept
    auto relIt = relationships.begin();
    while (relIt != relationships.end()) {
        if ((*relIt)->connectsTo(conceptId)) {
            std::string relationshipId = (*relIt)->getId();
            relIt = relationships.erase(relIt);
            emit relationshipRemoved(QString::fromStdString(relationshipId));
        } else {
            ++relIt;
        }
    }
    
    // Then remove the concept itself
    auto it = std::find_if(concepts.begin(), concepts.end(),
        [&conceptId](const std::unique_ptr<Concept>& concept) {
            return concept->getId() == conceptId;
        });
    if (it != concepts.end()) {
        concepts.erase(it);
        emit conceptRemoved(QString::fromStdString(conceptId));
        notifyChange(ModelChangeEvent(ChangeType::CONCEPT_REMOVED, conceptId));
    }
}

Concept* MentalModel::getConcept(const std::string& conceptId) {
    auto it = std::find_if(concepts.begin(), concepts.end(),
        [&conceptId](const std::unique_ptr<Concept>& concept) {
            return concept->getId() == conceptId;
        });
    return (it != concepts.end()) ? it->get() : nullptr;
}

const Concept* MentalModel::getConcept(const std::string& conceptId) const {
    auto it = std::find_if(concepts.begin(), concepts.end(),
        [&conceptId](const std::unique_ptr<Concept>& concept) {
            return concept->getId() == conceptId;
        });
    return (it != concepts.end()) ? it->get() : nullptr;
}

const std::vector<std::unique_ptr<Concept>>& MentalModel::getConcepts() const {
    return concepts;
}

// Relationship management
void MentalModel::addRelationship(std::unique_ptr<Relationship> relationship) {
    if (!relationship) return;
    // Validate that both concepts exist
    if (!getConcept(relationship->getSourceConceptId()) || 
        !getConcept(relationship->getTargetConceptId())) {
        return; // Don't add relationship if concepts don't exist
    }
    std::string relationshipId = relationship->getId();
    relationships.push_back(std::move(relationship));
    emit relationshipAdded(QString::fromStdString(relationshipId));
    notifyChange(ModelChangeEvent(ChangeType::RELATIONSHIP_ADDED, relationshipId));
}

void MentalModel::removeRelationship(const std::string& relationshipId) {
    auto it = std::find_if(relationships.begin(), relationships.end(),
        [&relationshipId](const std::unique_ptr<Relationship>& relationship) {
            return relationship->getId() == relationshipId;
        });
    if (it != relationships.end()) {
        relationships.erase(it);
        emit relationshipRemoved(QString::fromStdString(relationshipId));
        notifyChange(ModelChangeEvent(ChangeType::RELATIONSHIP_REMOVED, relationshipId));
    }
}

Relationship* MentalModel::getRelationship(const std::string& relationshipId) {
    auto it = std::find_if(relationships.begin(), relationships.end(),
        [&relationshipId](const std::unique_ptr<Relationship>& relationship) {
            return relationship->getId() == relationshipId;
        });
    return (it != relationships.end()) ? it->get() : nullptr;
}

const Relationship* MentalModel::getRelationship(const std::string& relationshipId) const {
    auto it = std::find_if(relationships.begin(), relationships.end(),
        [&relationshipId](const std::unique_ptr<Relationship>& relationship) {
            return relationship->getId() == relationshipId;
        });
    return (it != relationships.end()) ? it->get() : nullptr;
}

const std::vector<std::unique_ptr<Relationship>>& MentalModel::getRelationships() const {
    return relationships;
}

// Graph operations
std::vector<Concept*> MentalModel::getConnectedConcepts(const std::string& conceptId) {
    std::vector<Concept*> connected;
    for (const auto& relationship : relationships) {
        if (relationship->connectsTo(conceptId)) {
            std::string otherConceptId = relationship->getOtherConcept(conceptId);
            if (!otherConceptId.empty()) {
                Concept* otherConcept = getConcept(otherConceptId);
                if (otherConcept) {
                    connected.push_back(otherConcept);
                }
            }
        }
    }
    return connected;
}

std::vector<const Concept*> MentalModel::getConnectedConcepts(const std::string& conceptId) const {
    std::vector<const Concept*> connected;
    for (const auto& relationship : relationships) {
        if (relationship->connectsTo(conceptId)) {
            std::string otherConceptId = relationship->getOtherConcept(conceptId);
            if (!otherConceptId.empty()) {
                const Concept* otherConcept = getConcept(otherConceptId);
                if (otherConcept) {
                    connected.push_back(otherConcept);
                }
            }
        }
    }
    return connected;
}

std::vector<Relationship*> MentalModel::getConceptRelationships(const std::string& conceptId) {
    std::vector<Relationship*> conceptRelationships;
    for (const auto& relationship : relationships) {
        if (relationship->connectsTo(conceptId)) {
            conceptRelationships.push_back(relationship.get());
        }
    }
    return conceptRelationships;
}

std::vector<const Relationship*> MentalModel::getConceptRelationships(const std::string& conceptId) const {
    std::vector<const Relationship*> conceptRelationships;
    for (const auto& relationship : relationships) {
        if (relationship->connectsTo(conceptId)) {
            conceptRelationships.push_back(relationship.get());
        }
    }
    return conceptRelationships;
}

bool MentalModel::areConnected(const std::string& concept1Id, const std::string& concept2Id) const {
    for (const auto& relationship : relationships) {
        if (relationship->connects(concept1Id, concept2Id)) {
            return true;
        }
    }
    return false;
}

// graph operations
std::vector<std::string> MentalModel::findShortestPath(const std::string& startConceptId, 
                                                      const std::string& endConceptId) {
    if (startConceptId == endConceptId) {
        return {startConceptId};
    }
    
    // BFS to find shortest path
    std::queue<std::string> queue;
    std::set<std::string> visited;
    std::map<std::string, std::string> parent;
    
    queue.push(startConceptId);
    visited.insert(startConceptId);
    
    while (!queue.empty()) {
        std::string current = queue.front();
        queue.pop();
        
        if (current == endConceptId) {
            // Reconstruct path
            std::vector<std::string> path;
            std::string node = endConceptId;
            while (node != startConceptId) {
                path.push_back(node);
                node = parent[node];
            }
            path.push_back(startConceptId);
            std::reverse(path.begin(), path.end());
            return path;
        }
        
        // Explore neighbors
        auto connectedConcepts = getConnectedConcepts(current);
        for (const auto& concept : connectedConcepts) {
            std::string conceptId = concept->getId();
            if (visited.find(conceptId) == visited.end()) {
                visited.insert(conceptId);
                parent[conceptId] = current;
                queue.push(conceptId);
            }
        }
    }
    return {}; // No path found
}

std::vector<Concept*> MentalModel::getOrphanedConcepts() {
    std::vector<Concept*> orphaned;
    for (const auto& concept : concepts) {
        if (getConceptRelationships(concept->getId()).empty()) {
            orphaned.push_back(concept.get());
        }
    }
    return orphaned;
}

double MentalModel::getConceptImportance(const std::string& conceptId) {
    auto conceptRelationships = getConceptRelationships(conceptId);
    if (conceptRelationships.empty()) {
        return 0.0;
    }
    
    // Simple importance based on number of connections and their weights
    double importance = 0.0;
    for (const auto& rel : conceptRelationships) {
        importance += rel->getWeight();
    }
    
    // Normalize by total number of concepts to get relative importance
    return importance / static_cast<double>(concepts.size());
}

// Model properties
const std::string& MentalModel::getModelName() const {
    return modelName;
}

void MentalModel::setModelName(const std::string& name) {
    modelName = name;
}

size_t MentalModel::getConceptCount() const {
    return concepts.size();
}

size_t MentalModel::getRelationshipCount() const {
    return relationships.size();
}

void MentalModel::clear() {
    concepts.clear();
    relationships.clear();
    notifyChange(ModelChangeEvent(ChangeType::MODEL_CLEARED, "all"));
}

bool MentalModel::isEmpty() const {
    return concepts.empty() && relationships.empty();
}

// Model validation
bool MentalModel::isValid() const {
    // Check that all relationships reference existing concepts
    for (const auto& relationship : relationships) {
        if (!getConcept(relationship->getSourceConceptId()) || 
            !getConcept(relationship->getTargetConceptId())) {
            return false;
        }
    }
    
    // Check for duplicate concept IDs
    std::set<std::string> conceptIds;
    for (const auto& concept : concepts) {
        if (conceptIds.find(concept->getId()) != conceptIds.end()) {
            return false; // Duplicate ID found
        }
        conceptIds.insert(concept->getId());
    }
    
    // Check for duplicate relationship IDs
    std::set<std::string> relationshipIds;
    for (const auto& relationship : relationships) {
        if (relationshipIds.find(relationship->getId()) != relationshipIds.end()) {
            return false; // Duplicate ID found
        }
        relationshipIds.insert(relationship->getId());
    }
    
    return true;
}

std::vector<std::string> MentalModel::getValidationErrors() const {
    std::vector<std::string> errors;
    
    // Check relationships reference existing concepts
    for (const auto& relationship : relationships) {
        if (!getConcept(relationship->getSourceConceptId())) {
            errors.push_back("Relationship " + relationship->getId() + 
                           " references non-existent source concept " + 
                           relationship->getSourceConceptId());
        }
        if (!getConcept(relationship->getTargetConceptId())) {
            errors.push_back("Relationship " + relationship->getId() + 
                           " references non-existent target concept " + 
                           relationship->getTargetConceptId());
        }
    }
    return errors;
}

// Statistics
ModelStatistics MentalModel::getStatistics() const {
    ModelStatistics stats;
    stats.conceptCount = concepts.size();
    stats.relationshipCount = relationships.size();
    stats.orphanedConceptCount = 0;
    stats.averageConnections = 0.0;
    stats.maxConnections = 0;
    stats.minConnections = concepts.empty() ? 0 : SIZE_MAX;
    
    if (!concepts.empty()) {
        size_t totalConnections = 0;
        for (const auto& concept : concepts) {
            size_t connections = getConceptRelationships(concept->getId()).size();
            totalConnections += connections;
            if (connections == 0) {
                stats.orphanedConceptCount++;
            }
            if (connections > stats.maxConnections) {
                stats.maxConnections = connections;
            }
            if (connections < stats.minConnections) {
                stats.minConnections = connections;
            }
        }
        stats.averageConnections = static_cast<double>(totalConnections) / concepts.size();
    }
    
    return stats;
}

// JSON serialization (basic implementation)
std::string MentalModel::toJson() const {
    std::stringstream ss;
    ss << "{";
    ss << "\"modelName\":\"" << modelName << "\",";
    ss << "\"concepts\":[";
    for (size_t i = 0; i < concepts.size(); ++i) {
        if (i > 0) ss << ",";
        ss << concepts[i]->toJson();
    }
    ss << "],\"relationships\":[";
    for (size_t i = 0; i < relationships.size(); ++i) {
        if (i > 0) ss << ",";
        ss << relationships[i]->toJson();
    }
    ss << "]}";
    return ss.str();
}

std::unique_ptr<MentalModel> MentalModel::fromJson(const std::string& json) {
    // TODO: Implement proper JSON parsing in Task 4.1
    // For now, return nullptr to indicate not implemented
    return nullptr;
}

void MentalModel::notifyChange(const ModelChangeEvent& event) {
    emit modelChanged(event);
}

} // namespace qlink