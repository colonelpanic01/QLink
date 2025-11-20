#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../common/DataStructures.h"

namespace qlink {

/**
 * Represents a concept node in the mental model
 */
class Concept {
private:
    std::string id;
    std::string name;
    std::string description;
    std::vector<std::string> tags;
    Position position;

public:
    // Constructors
    Concept(const std::string& name, const std::string& description = "");
    Concept(const std::string& id, const std::string& name, const std::string& description);
    
    // Getters
    const std::string& getId() const { return id; }
    const std::string& getName() const { return name; }
    const std::string& getDescription() const { return description; }
    const std::vector<std::string>& getTags() const { return tags; }
    const Position& getPosition() const { return position; }
    
    // Setters
    void setName(const std::string& name);
    void setDescription(const std::string& description);
    void setPosition(const Position& position);
    
    // Tag management
    void addTag(const std::string& tag);
    void removeTag(const std::string& tag);
    bool hasTag(const std::string& tag) const;
    
    // Utility methods
    bool operator==(const Concept& other) const;
    std::string toString() const;
    
    // JSON serialization
    std::string toJson() const;

private:
    static std::string generateId();
};

} // namespace qlink