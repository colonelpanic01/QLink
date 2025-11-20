#include "Relationship.h"
#include <sstream>
#include <random>
#include <iomanip>

namespace qlink {

Relationship::Relationship(const std::string& sourceId, const std::string& targetId,
                          const std::string& type, bool directed, double weight)
    : id(generateId()), sourceConceptId(sourceId), targetConceptId(targetId),
      type(type), isDirected(directed), weight(weight) {
}

Relationship::Relationship(const std::string& id, const std::string& sourceId, const std::string& targetId,
                          const std::string& type, bool directed, double weight)
    : id(id), sourceConceptId(sourceId), targetConceptId(targetId),
      type(type), isDirected(directed), weight(weight) {
}

void Relationship::setType(const std::string& type) {
    this->type = type;
}

void Relationship::setWeight(double weight) {
    this->weight = weight;
}

void Relationship::setDirected(bool directed) {
    this->isDirected = directed;
}

bool Relationship::connects(const std::string& concept1, const std::string& concept2) const {
    if (isDirected) {
        return (sourceConceptId == concept1 && targetConceptId == concept2);
    } else {
        return (sourceConceptId == concept1 && targetConceptId == concept2) ||
               (sourceConceptId == concept2 && targetConceptId == concept1);
    }
}

bool Relationship::connectsTo(const std::string& conceptId) const {
    return sourceConceptId == conceptId || targetConceptId == conceptId;
}

std::string Relationship::getOtherConcept(const std::string& conceptId) const {
    if (sourceConceptId == conceptId) {
        return targetConceptId;
    } else if (targetConceptId == conceptId) {
        return sourceConceptId;
    }
    return "";
}

bool Relationship::operator==(const Relationship& other) const {
    return id == other.id;
}

std::string Relationship::toString() const {
    std::ostringstream oss;
    oss << "Relationship[" << id << "]: " << sourceConceptId;
    if (isDirected) {
        oss << " -> ";
    } else {
        oss << " <-> ";
    }
    oss << targetConceptId;
    if (!type.empty()) {
        oss << " (" << type << ")";
    }
    return oss.str();
}

std::string Relationship::toJson() const {
    return "{}";
}

std::unique_ptr<Relationship> Relationship::fromJson(const std::string& json) {
    return nullptr;
}

std::string Relationship::generateId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    
    std::ostringstream oss;
    oss << "rel_";
    for (int i = 0; i < 8; ++i) {
        oss << std::hex << dis(gen);
    }
    return oss.str();
}

} // namespace qlink