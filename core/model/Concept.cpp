#include "Concept.h"
#include <algorithm>
#include <sstream>
#include <random>
#include <iomanip>

namespace qlink {

Concept::Concept(const std::string& name, const std::string& description)
    : id(generateId()), name(name), description(description), position(0.0, 0.0) {
}

Concept::Concept(const std::string& id, const std::string& name, const std::string& description)
    : id(id), name(name), description(description), position(0.0, 0.0) {
}

void Concept::setName(const std::string& name) {
    this->name = name;
}

void Concept::setDescription(const std::string& description) {
    this->description = description;
}

void Concept::setPosition(const Position& position) {
    this->position = position;
}

void Concept::addTag(const std::string& tag) {
    if (!hasTag(tag)) {
        tags.push_back(tag);
    }
}

void Concept::removeTag(const std::string& tag) {
    tags.erase(std::remove(tags.begin(), tags.end(), tag), tags.end());
}

bool Concept::hasTag(const std::string& tag) const {
    return std::find(tags.begin(), tags.end(), tag) != tags.end();
}

bool Concept::operator==(const Concept& other) const {
    return id == other.id;
}

std::string Concept::toString() const {
    std::ostringstream oss;
    oss << "Concept[" << id << "]: " << name;
    if (!description.empty()) {
        oss << " - " << description;
    }
    return oss.str();
}

std::string Concept::toJson() const {
    std::ostringstream oss;
    oss << "{";
    oss << "\"id\":\"" << id << "\",";
    oss << "\"name\":\"" << name << "\",";
    oss << "\"description\":\"" << description << "\",";
    oss << "\"position\":{\"x\":" << position.x << ",\"y\":" << position.y << "},";
    oss << "\"tags\":[";
    for (size_t i = 0; i < tags.size(); ++i) {
        if (i > 0) oss << ",";
        oss << "\"" << tags[i] << "\"";
    }
    oss << "]}";
    return oss.str();
}

std::string Concept::generateId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    
    std::ostringstream oss;
    oss << "concept_";
    for (int i = 0; i < 8; ++i) {
        oss << std::hex << dis(gen);
    }
    return oss.str();
}

} // namespace qlink