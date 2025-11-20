#pragma once

#include <string>
#include <memory>

namespace qlink {

/**
 * Represents a relationship edge between two concepts
 */
class Relationship {
private:
    std::string id;
    std::string sourceConceptId;
    std::string targetConceptId;
    std::string type;
    bool isDirected;
    double weight;

public:
    // Constructors
    Relationship(const std::string& sourceId, const std::string& targetId, 
                const std::string& type = "", bool directed = false, double weight = 1.0);
    Relationship(const std::string& id, const std::string& sourceId, const std::string& targetId,
                const std::string& type = "", bool directed = false, double weight = 1.0);
    
    // Getters
    const std::string& getId() const { return id; }
    const std::string& getSourceConceptId() const { return sourceConceptId; }
    const std::string& getTargetConceptId() const { return targetConceptId; }
    const std::string& getType() const { return type; }
    bool getIsDirected() const { return isDirected; }
    double getWeight() const { return weight; }
    
    // Setters
    void setType(const std::string& type);
    void setWeight(double weight);
    void setDirected(bool directed);
    
    // Utility methods
    bool connects(const std::string& concept1, const std::string& concept2) const;
    bool connectsTo(const std::string& conceptId) const;
    std::string getOtherConcept(const std::string& conceptId) const;
    bool operator==(const Relationship& other) const;
    std::string toString() const;
    
    // JSON serialization
    std::string toJson() const;

private:
    static std::string generateId();
};

} // namespace qlink