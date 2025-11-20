#include "JaccardCoefficientPredictor.h"
#include "../model/MentalModel.h"

namespace qlink {

JaccardCoefficientPredictor::JaccardCoefficientPredictor(QObject *parent)
    : IGraphLinkPredictor(parent) {
}

std::vector<LinkSuggestion> JaccardCoefficientPredictor::predictLinks(const MentalModel& model, int maxSuggestions) {
    const auto& concepts = model.getConcepts();
    
    if (concepts.size() < 2) {
        return std::vector<LinkSuggestion>();
    }
    
    // Convert to igraph
    igraph_t graph;
    std::map<std::string, int> conceptToVertex;
    convertToIGraph(model, &graph, conceptToVertex);
    
    // Calculate Jaccard similarity using igraph
    igraph_matrix_t similarity;
    igraph_matrix_init(&similarity, 0, 0);
    
    // Use igraph's built-in Jaccard similarity  
    igraph_similarity_jaccard(&graph, &similarity, igraph_vss_all(), igraph_vss_all(), IGRAPH_ALL, IGRAPH_NO_LOOPS);
    
    // Convert to suggestions
    auto suggestions = convertSimilarityToSuggestions(&similarity, conceptToVertex, model, maxSuggestions, "Jaccard Coefficient");
    
    // Cleanup
    igraph_matrix_destroy(&similarity);
    cleanupIGraph(&graph);
    
    return suggestions;
}

std::string JaccardCoefficientPredictor::getAlgorithmName() const {
    return "Jaccard Coefficient";
}

std::string JaccardCoefficientPredictor::getDescription() const {
    return "Predicts links using igraph's Jaccard coefficient: |intersection| / |union| of neighbors";
}

} // namespace qlink