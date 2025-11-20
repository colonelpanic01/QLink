#include "JaccardCoefficientPredictor.h"
#include <algorithm>
#include <set>

namespace qlink {

JaccardCoefficientPredictor::JaccardCoefficientPredictor(QObject *parent)
    : ILinkPredictor(parent) {
}

std::vector<LinkSuggestion> JaccardCoefficientPredictor::predictLinks(const MentalModel& model, int maxSuggestions) {
    std::vector<LinkSuggestion> suggestions;
    const auto& concepts = model.getConcepts();
    
    if (concepts.size() < 2) {
        return suggestions;
    }
    
    std::vector<std::pair<double, std::pair<std::string, std::string>>> scoredPairs;
    
    // Calculate Jaccard coefficient for all pairs of unconnected concepts
    for (size_t i = 0; i < concepts.size(); ++i) {
        for (size_t j = i + 1; j < concepts.size(); ++j) {
            const std::string& concept1Id = concepts[i]->getId();
            const std::string& concept2Id = concepts[j]->getId();
            
            // Skip if concepts are already connected
            if (model.areConnected(concept1Id, concept2Id)) {
                continue;
            }
            
            double jaccardCoeff = calculateJaccardCoefficient(model, concept1Id, concept2Id);
            
            if (jaccardCoeff > 0.0) {
                scoredPairs.emplace_back(jaccardCoeff, std::make_pair(concept1Id, concept2Id));
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
        double confidence = pair.first;
        const std::string& sourceId = pair.second.first;
        const std::string& targetId = pair.second.second;
        
        std::string explanation = "Jaccard coefficient: " + std::to_string(confidence);
        
        suggestions.emplace_back(sourceId, targetId, "relates_to", confidence, explanation);
    }
    
    return suggestions;
}

std::string JaccardCoefficientPredictor::getAlgorithmName() const {
    return "Jaccard Coefficient";
}

std::string JaccardCoefficientPredictor::getDescription() const {
    return "Predicts links using Jaccard coefficient: |intersection| / |union| of neighbors";
}

double JaccardCoefficientPredictor::calculateJaccardCoefficient(const MentalModel& model, 
                                                               const std::string& concept1Id, 
                                                               const std::string& concept2Id) {
    std::set<std::string> neighbors1 = getNeighbors(model, concept1Id);
    std::set<std::string> neighbors2 = getNeighbors(model, concept2Id);
    
    if (neighbors1.empty() && neighbors2.empty()) {
        return 0.0;
    }
    
    // Calculate intersection
    std::set<std::string> intersection;
    std::set_intersection(neighbors1.begin(), neighbors1.end(),
                         neighbors2.begin(), neighbors2.end(),
                         std::inserter(intersection, intersection.begin()));
    
    // Calculate union  
    std::set<std::string> unionSet;
    std::set_union(neighbors1.begin(), neighbors1.end(),
                   neighbors2.begin(), neighbors2.end(),
                   std::inserter(unionSet, unionSet.begin()));
    
    if (unionSet.empty()) {
        return 0.0;
    }
    
    return static_cast<double>(intersection.size()) / static_cast<double>(unionSet.size());
}

std::set<std::string> JaccardCoefficientPredictor::getNeighbors(const MentalModel& model, const std::string& conceptId) {
    std::set<std::string> neighbors;
    const auto& relationships = model.getRelationships();
    
    for (const auto& relationship : relationships) {
        if (relationship->getSourceConceptId() == conceptId) {
            neighbors.insert(relationship->getTargetConceptId());
        } else if (relationship->getTargetConceptId() == conceptId) {
            neighbors.insert(relationship->getSourceConceptId());
        }
    }
    
    return neighbors;
}

} // namespace qlink