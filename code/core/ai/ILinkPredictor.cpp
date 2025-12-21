#include "ILinkPredictor.h"
#include "CommonNeighborPredictor.h"
#include "JaccardCoefficientPredictor.h"
#include "PreferentialAttachmentPredictor.h"
#include "../model/MentalModel.h"
#include <stdexcept>
#include <algorithm>

namespace qlink {

void IGraphLinkPredictor::convertToIGraph(const MentalModel& model, igraph_t* graph, std::map<std::string, int>& conceptToVertex) {
    const auto& concepts = model.getConcepts();
    const auto& relationships = model.getRelationships();
    
    conceptToVertex.clear();
    for (size_t i = 0; i < concepts.size(); ++i) {
        conceptToVertex[concepts[i]->getId()] = static_cast<int>(i);
    }
    
    // Initialize graph and add edges
    igraph_empty(graph, static_cast<int>(concepts.size()), IGRAPH_UNDIRECTED);
    
    igraph_vector_int_t edges;
    igraph_vector_int_init(&edges, 0);
    
    for (const auto& relationship : relationships) {
        auto sourceIt = conceptToVertex.find(relationship->getSourceConceptId());
        auto targetIt = conceptToVertex.find(relationship->getTargetConceptId());
        
        if (sourceIt != conceptToVertex.end() && targetIt != conceptToVertex.end()) {
            igraph_vector_int_push_back(&edges, sourceIt->second);
            igraph_vector_int_push_back(&edges, targetIt->second);
        }
    }
    
    igraph_add_edges(graph, &edges, nullptr);
    igraph_vector_int_destroy(&edges);
}

std::vector<LinkSuggestion> IGraphLinkPredictor::convertSimilarityToSuggestions(
    const igraph_matrix_t* similarity,
    const std::map<std::string, int>& conceptToVertex,
    const MentalModel& model,
    int maxSuggestions,
    const std::string& algorithmName) {
    
    std::vector<LinkSuggestion> suggestions;
    std::vector<std::pair<double, std::pair<std::string, std::string>>> scoredPairs;
    
    // Create reverse mapping from vertex to concept ID
    std::map<int, std::string> vertexToConcept;
    for (const auto& pair : conceptToVertex) {
        vertexToConcept[pair.second] = pair.first;
    }
    
    // Extract similarities for unconnected pairs
    int numVertices = static_cast<int>(conceptToVertex.size());
    for (int i = 0; i < numVertices; ++i) {
        for (int j = i + 1; j < numVertices; ++j) {
            const std::string& concept1Id = vertexToConcept[i];
            const std::string& concept2Id = vertexToConcept[j];
            
            // Skip if already connected
            if (model.areConnected(concept1Id, concept2Id)) {
                continue;
            }
            
            double score = MATRIX(*similarity, i, j);
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
    
    // Find max score for normalization
    double maxScore = scoredPairs.empty() ? 1.0 : scoredPairs[0].first;
    
    for (int i = 0; i < count; ++i) {
        const auto& pair = scoredPairs[i];
        double rawScore = pair.first;
        const std::string& sourceId = pair.second.first;
        const std::string& targetId = pair.second.second;
        
        // Normalize confidence to 0.3-1.0 range for better visibility
        double confidence = 0.3 + (rawScore / maxScore) * 0.7;
        
        std::string explanation = algorithmName + " score: " + std::to_string(rawScore) + 
                                " (normalized: " + std::to_string(confidence) + ")";
        suggestions.emplace_back(sourceId, targetId, "relates_to", confidence, explanation, algorithmName);
    }
    
    return suggestions;
}

void IGraphLinkPredictor::cleanupIGraph(igraph_t* graph) {
    igraph_destroy(graph);
}

std::unique_ptr<ILinkPredictor> LinkPredictorFactory::createPredictor(LinkPredictorFactory::AlgorithmType type) {
    switch (type) {
        case LinkPredictorFactory::AlgorithmType::COMMON_NEIGHBORS:
            return std::make_unique<CommonNeighborPredictor>();
        case LinkPredictorFactory::AlgorithmType::JACCARD_COEFFICIENT:
            return std::make_unique<JaccardCoefficientPredictor>();
        case LinkPredictorFactory::AlgorithmType::PREFERENTIAL_ATTACHMENT:
            return std::make_unique<PreferentialAttachmentPredictor>();
        default:
            throw std::runtime_error("Unknown algorithm type");
    }
}

std::vector<LinkPredictorFactory::AlgorithmType> LinkPredictorFactory::getAvailableAlgorithms() {
    return {
        LinkPredictorFactory::AlgorithmType::COMMON_NEIGHBORS,
        LinkPredictorFactory::AlgorithmType::JACCARD_COEFFICIENT,
        LinkPredictorFactory::AlgorithmType::PREFERENTIAL_ATTACHMENT
    };
}

std::string LinkPredictorFactory::getAlgorithmName(LinkPredictorFactory::AlgorithmType type) {
    switch (type) {
        case LinkPredictorFactory::AlgorithmType::COMMON_NEIGHBORS:
            return "Common Neighbors";
        case LinkPredictorFactory::AlgorithmType::JACCARD_COEFFICIENT:
            return "Jaccard Coefficient";
        case LinkPredictorFactory::AlgorithmType::PREFERENTIAL_ATTACHMENT:
            return "Preferential Attachment";
        default:
            return "Unknown Algorithm";
    }
}

} // namespace qlink