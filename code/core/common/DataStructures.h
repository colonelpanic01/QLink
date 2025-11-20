#pragma once

#include <string>
#include <vector>

namespace qlink {

/**
 * Represents a 2D position to visualize our concepts/nodes 
 */
struct Position {
    double x;
    double y;
    
    Position(double x = 0.0, double y = 0.0) : x(x), y(y) {}
    
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

/**
 * Represents a link prediction suggestion
 */
struct LinkSuggestion {
    std::string sourceConceptId;
    std::string targetConceptId;
    std::string suggestedType;
    double confidence;  // 0.0 to 1.0
    std::string explanation;
    
    LinkSuggestion(const std::string& source, const std::string& target, 
                   const std::string& type, double confidence, 
                   const std::string& explanation = "")
        : sourceConceptId(source), targetConceptId(target), 
          suggestedType(type), confidence(confidence), explanation(explanation) {}
};

/**
 * This enum reps is for different types of model changes
 */
enum class ChangeType {
    CONCEPT_ADDED,
    CONCEPT_REMOVED,
    CONCEPT_MODIFIED,
    RELATIONSHIP_ADDED,
    RELATIONSHIP_REMOVED,
    RELATIONSHIP_MODIFIED,
    MODEL_CLEARED
};

/**
 * Event structure for any model changes
 */
struct ModelChangeEvent {
    ChangeType type;
    std::string entityId;
    std::string details;
    
    ModelChangeEvent(ChangeType t, const std::string& id, const std::string& det = "")
        : type(t), entityId(id), details(det) {}
};

/**
 * Relationship strength categories
 */
enum class RelationshipStrength {
    VERY_WEAK,
    WEAK,
    MEDIUM,
    STRONG
};

/**
 * Model statistics for analysis
 */
struct ModelStatistics {
    size_t conceptCount = 0;
    size_t relationshipCount = 0;
    size_t orphanedConceptCount = 0;
    double averageConnections = 0.0;
    size_t maxConnections = 0;
    size_t minConnections = 0;
};

/**
 * Common relationship types
 */
namespace RelationshipTypes {
    const std::string CAUSES = "causes";
    const std::string REQUIRES = "requires";
    const std::string CONTRADICTS = "contradicts";
    const std::string SIMILAR_TO = "similar_to";
    const std::string PART_OF = "part_of";
    const std::string DEPENDS_ON = "depends_on";
    const std::string TRANSFORMS_TO = "transforms_to";
}

} // namespace qlink