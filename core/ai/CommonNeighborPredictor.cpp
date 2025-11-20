#include "CommonNeighborPredictor.h"
#include "../model/MentalModel.h"
#include <algorithm>
#include <unordered_set>

namespace qlink {

CommonNeighborPredictor::CommonNeighborPredictor(QObject *parent)
    : ILinkPredictor(parent) {
}

std::vector<LinkSuggestion> CommonNeighborPredictor::predictLinks(const MentalModel& model, int maxSuggestions) {
    std::vector<LinkSuggestion> suggestions;
    const auto& concepts = model.getConcepts();
    
    if (concepts.size() < 2) {
        return suggestions;
    }

    // Calculate common neighbor scores for all concept pairs
    std::vector<std::pair<int, std::pair<std::string, std::string>>> scoredPairs;
    
    for (size_t i = 0; i < concepts.size(); ++i) {
        for (size_t j = i + 1; j < concepts.size(); ++j) {
            const auto& concept1 = concepts[i];
            const auto& concept2 = concepts[j];
            
            // Skip if already connected
            if (model.areConnected(concept1->getId(), concept2->getId())) {
                continue;
            }
            
            // Calculate common neighbors
            int commonCount = calculateCommonNeighbors(model, concept1->getId(), concept2->getId());
            if (commonCount > 0) {
                scoredPairs.emplace_back(commonCount, std::make_pair(concept1->getId(), concept2->getId()));
            }
        }
    }

    // Sort by score (highest first)
    std::sort(scoredPairs.begin(), scoredPairs.end(), 
              [](const auto& a, const auto& b) { return a.first > b.first; });

    // Convert top results to LinkSuggestions
    int count = std::min(maxSuggestions, static_cast<int>(scoredPairs.size()));
    suggestions.reserve(count);
    
    for (int i = 0; i < count; ++i) {
        const auto& pair = scoredPairs[i];
        int commonCount = pair.first;
        const std::string& sourceId = pair.second.first;
        const std::string& targetId = pair.second.second;
        
        // Convert count to confidence (0.0 to 1.0)
        double confidence = std::min(1.0, commonCount / 5.0); // Max confidence at 5+ common neighbors
        
        std::string explanation = "Has " + std::to_string(commonCount) + " common neighbor(s)";
        
        suggestions.emplace_back(sourceId, targetId, "relates_to", confidence, explanation);
    }

    return suggestions;
}

int CommonNeighborPredictor::calculateCommonNeighbors(const MentalModel& model,
                                                     const std::string& concept1Id,
                                                     const std::string& concept2Id) {
    auto neighbors1 = getNeighbors(model, concept1Id);
    auto neighbors2 = getNeighbors(model, concept2Id);
    
    if (neighbors1.empty() || neighbors2.empty()) {
        return 0;
    }

    // Count common neighbors using set intersection
    std::unordered_set<std::string> set1(neighbors1.begin(), neighbors1.end());
    int commonCount = 0;
    for (const auto& neighbor : neighbors2) {
        if (set1.find(neighbor) != set1.end()) {
            commonCount++;
        }
    }
    
    return commonCount;
}

std::vector<std::string> CommonNeighborPredictor::getNeighbors(const MentalModel& model, 
                                                              const std::string& conceptId) {
    std::vector<std::string> neighbors;
    const auto& relationships = model.getRelationships();
    
    for (const auto& relationship : relationships) {
        if (relationship->getSourceConceptId() == conceptId) {
            neighbors.push_back(relationship->getTargetConceptId());
        } else if (relationship->getTargetConceptId() == conceptId) {
            neighbors.push_back(relationship->getSourceConceptId());
        }
    }
    
    return neighbors;
}

} // namespace qlink

