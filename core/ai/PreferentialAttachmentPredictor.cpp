#include "PreferentialAttachmentPredictor.h"
#include <algorithm>

namespace qlink {

PreferentialAttachmentPredictor::PreferentialAttachmentPredictor(QObject *parent)
    : ILinkPredictor(parent) {
}

std::vector<LinkSuggestion> PreferentialAttachmentPredictor::predictLinks(const MentalModel& model, int maxSuggestions) {
    std::vector<LinkSuggestion> suggestions;
    const auto& concepts = model.getConcepts();
    
    if (concepts.size() < 2) {
        return suggestions;
    }
    
    std::vector<std::pair<double, std::pair<std::string, std::string>>> scoredPairs;
    
    // Calculate preferential attachment score for all pairs of unconnected concepts
    for (size_t i = 0; i < concepts.size(); ++i) {
        for (size_t j = i + 1; j < concepts.size(); ++j) {
            const std::string& concept1Id = concepts[i]->getId();
            const std::string& concept2Id = concepts[j]->getId();
            
            // Skip if concepts are already connected
            if (model.areConnected(concept1Id, concept2Id)) {
                continue;
            }
            
            double score = calculatePreferentialAttachmentScore(model, concept1Id, concept2Id);
            
            if (score > 0.0) {
                scoredPairs.emplace_back(score, std::make_pair(concept1Id, concept2Id));
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
        
        int degree1 = getDegree(model, sourceId);
        int degree2 = getDegree(model, targetId);
        
        std::string explanation = "Preferential attachment (degrees: " + 
                                 std::to_string(degree1) + " x " + std::to_string(degree2) + ")";
        
        suggestions.emplace_back(sourceId, targetId, "relates_to", confidence, explanation);
    }
    
    return suggestions;
}

std::string PreferentialAttachmentPredictor::getAlgorithmName() const {
    return "Preferential Attachment";
}

std::string PreferentialAttachmentPredictor::getDescription() const {
    return "Predicts links based on node degrees: score = degree(u) x degree(v)";
}

double PreferentialAttachmentPredictor::calculatePreferentialAttachmentScore(const MentalModel& model, 
                                                                           const std::string& concept1Id, 
                                                                           const std::string& concept2Id) {
    int degree1 = getDegree(model, concept1Id);
    int degree2 = getDegree(model, concept2Id);
    
    // Add 1 to handle isolated nodes, normalize to 0-1 range
    double score = static_cast<double>((degree1 + 1) * (degree2 + 1));
    return std::min(1.0, score / 100.0); // Normalize with reasonable upper bound
}

int PreferentialAttachmentPredictor::getDegree(const MentalModel& model, const std::string& conceptId) {
    int degree = 0;
    const auto& relationships = model.getRelationships();
    
    for (const auto& relationship : relationships) {
        if (relationship->getSourceConceptId() == conceptId || 
            relationship->getTargetConceptId() == conceptId) {
            degree++;
        }
    }
    
    return degree;
}

} // namespace qlink