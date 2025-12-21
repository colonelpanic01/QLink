#include "CommandFactory.h"
#include "Commands.h"
#include "../model/MentalModel.h"
#include <regex>
#include <algorithm>
#include <cctype>

namespace qlink {

// Helper function to trim whitespace
static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

// Helper function to convert to lowercase
static std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

// Helper to find concept by name (case-insensitive)
static std::string findConceptIdByName(MentalModel* model, const std::string& name) {
    std::string lowerName = toLower(name);
    for (const auto& concept : model->getConcepts()) {
        if (toLower(concept->getName()) == lowerName) {
            return concept->getId();
        }
    }
    return "";
}

std::unique_ptr<ICommand> CommandFactory::createCommand(const std::string& input, MentalModel* model) {
    if (!model) return nullptr;
    
    std::string trimmedInput = trim(input);
    if (trimmedInput.empty()) return nullptr;
    
    std::string lowerInput = toLower(trimmedInput);
    
    // Pattern 1: "add concept <name>" or "create concept <name>"
    std::regex addConceptRegex(R"((add|create)\s+concept\s+[\"']?([^\"'\n]+?)[\"']?(?:\s+(?:with|having)\s+(?:description|desc)\s+[\"']?([^\"'\n]+?)[\"']?)?$)", std::regex::icase);
    std::smatch match;
    
    if (std::regex_search(trimmedInput, match, addConceptRegex)) {
        std::string conceptName = trim(match[2].str());
        std::string description = match[3].matched ? trim(match[3].str()) : "";
        return std::make_unique<AddConceptCommand>(model, conceptName, description);
    }
    
    // Pattern 2: "remove concept <name>" or "delete concept <name>"
    std::regex removeConceptRegex(R"((remove|delete)\s+concept\s+[\"']?([^\"'\n]+?)[\"']?$)", std::regex::icase);
    if (std::regex_search(trimmedInput, match, removeConceptRegex)) {
        std::string conceptName = trim(match[2].str());
        std::string conceptId = findConceptIdByName(model, conceptName);
        if (!conceptId.empty()) {
            return std::make_unique<RemoveConceptCommand>(model, conceptId);
        }
    }
    
    // Pattern 3: "connect <concept1> to <concept2>" or "link <concept1> to <concept2>"
    std::regex connectRegex(R"((connect|link|relate)\s+[\"']?([^\"'\n]+?)[\"']?\s+(?:to|with|and)\s+[\"']?([^\"'\n]+?)[\"']?(?:\s+(?:as|type|with type)\s+[\"']?([^\"'\n]+?)[\"']?)?(?:\s+(directed|undirected))?$)", std::regex::icase);
    if (std::regex_search(trimmedInput, match, connectRegex)) {
        std::string concept1 = trim(match[2].str());
        std::string concept2 = trim(match[3].str());
        std::string type = match[4].matched ? trim(match[4].str()) : "relates_to";
        bool directed = match[5].matched && toLower(match[5].str()) == "directed";
        
        std::string id1 = findConceptIdByName(model, concept1);
        std::string id2 = findConceptIdByName(model, concept2);
        
        if (!id1.empty() && !id2.empty()) {
            return std::make_unique<CreateRelationshipCommand>(model, id1, id2, type, directed);
        }
    }
    
    // Pattern 4: "disconnect <concept1> from <concept2>" or "remove link between <concept1> and <concept2>"
    std::regex disconnectRegex(R"((disconnect|unlink|remove (?:link|relationship))\s+(?:between\s+)?[\"']?([^\"'\n]+?)[\"']?\s+(?:from|and)\s+[\"']?([^\"'\n]+?)[\"']?$)", std::regex::icase);
    if (std::regex_search(trimmedInput, match, disconnectRegex)) {
        std::string concept1 = trim(match[2].str());
        std::string concept2 = trim(match[3].str());
        
        std::string id1 = findConceptIdByName(model, concept1);
        std::string id2 = findConceptIdByName(model, concept2);
        
        if (!id1.empty() && !id2.empty()) {
            // Find the relationship between these two concepts
            for (const auto& rel : model->getRelationships()) {
                if (rel->connects(id1, id2)) {
                    return std::make_unique<DeleteRelationshipCommand>(model, rel->getId());
                }
            }
        }
    }
    
    // Pattern 5: Simple "add <name>" without "concept" keyword
    std::regex simpleAddRegex(R"(^add\s+[\"']?([^\"'\n]+?)[\"']?$)", std::regex::icase);
    if (std::regex_search(trimmedInput, match, simpleAddRegex)) {
        std::string conceptName = trim(match[1].str());
        // Avoid matching if it looks like it's trying to add something else
        if (toLower(conceptName).find("relationship") == std::string::npos &&
            toLower(conceptName).find("link") == std::string::npos) {
            return std::make_unique<AddConceptCommand>(model, conceptName, "");
        }
    }
    
    return nullptr;
}

bool CommandFactory::isValidCommand(const std::string& input) {
    std::string trimmedInput = trim(input);
    if (trimmedInput.empty()) return false;
    
    std::string lowerInput = toLower(trimmedInput);
    
    // Check for command keywords
    std::vector<std::string> keywords = {
        "add", "create", "remove", "delete", "connect", "link", 
        "disconnect", "unlink", "relate"
    };
    
    for (const auto& keyword : keywords) {
        if (lowerInput.find(keyword) == 0 || 
            lowerInput.find(" " + keyword + " ") != std::string::npos) {
            return true;
        }
    }
    
    return false;
}

std::string CommandFactory::getHelpText() {
    return "Natural Language Commands:\n\n"
           "Add a concept:\n"
           "  - add concept <name>\n"
           "  - create concept <name> with description <text>\n"
           "  - add <name>\n\n"
           "Remove a concept:\n"
           "  - remove concept <name>\n"
           "  - delete concept <name>\n\n"
           "Create a relationship:\n"
           "  - connect <concept1> to <concept2>\n"
           "  - link <concept1> and <concept2>\n"
           "  - relate <concept1> with <concept2> as <type>\n"
           "  - connect <concept1> to <concept2> directed\n\n"
           "Remove a relationship:\n"
           "  - disconnect <concept1> from <concept2>\n"
           "  - remove link between <concept1> and <concept2>\n\n"
           "Examples:\n"
           "  - add concept Machine Learning\n"
           "  - create concept AI with description Artificial Intelligence\n"
           "  - connect AI to Machine Learning\n"
           "  - remove concept AI\n";
}

} // namespace qlink