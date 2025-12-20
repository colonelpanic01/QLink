#include "PreferentialAttachmentPredictor.h"
#include "../model/MentalModel.h"

namespace qlink {

PreferentialAttachmentPredictor::PreferentialAttachmentPredictor(QObject *parent)
    : IGraphLinkPredictor(parent) {
}

std::vector<LinkSuggestion> PreferentialAttachmentPredictor::predictLinks(const MentalModel& model, int maxSuggestions) {
    const auto& concepts = model.getConcepts();
    
    if (concepts.size() < 2) {
        return std::vector<LinkSuggestion>();
    }
    
    // Convert to igraph
    igraph_t graph;
    std::map<std::string, int> conceptToVertex;
    convertToIGraph(model, &graph, conceptToVertex);
    
    // Calculate similarity matrix for preferential attachment
    igraph_matrix_t similarity;
    igraph_matrix_init(&similarity, 0, 0);
    
    int numVertices = igraph_vcount(&graph);
    igraph_matrix_resize(&similarity, numVertices, numVertices);
    igraph_matrix_fill(&similarity, 0.0);
    
    // Get degrees for all vertices
    igraph_vector_int_t degrees;
    igraph_vector_int_init(&degrees, numVertices);
    igraph_degree(&graph, &degrees, igraph_vss_all(), IGRAPH_ALL, IGRAPH_NO_LOOPS);
    
    // Calculate preferential attachment scores
    for (int i = 0; i < numVertices; ++i) {
        for (int j = i + 1; j < numVertices; ++j) {
            double degree_i = (double)VECTOR(degrees)[i];
            double degree_j = (double)VECTOR(degrees)[j];
            
            // Preferential attachment score = degree(i) * degree(j)
            // Add 1 to handle isolated nodes
            double score = (degree_i + 1) * (degree_j + 1);
            
            MATRIX(similarity, i, j) = score;
            MATRIX(similarity, j, i) = score;
        }
    }
    
    // Convert to suggestions  
    auto suggestions = convertSimilarityToSuggestions(&similarity, conceptToVertex, model, maxSuggestions, "Preferential Attachment");
    
    // Cleanup
    igraph_vector_int_destroy(&degrees);
    igraph_matrix_destroy(&similarity);
    cleanupIGraph(&graph);
    
    return suggestions;
}

std::string PreferentialAttachmentPredictor::getAlgorithmName() const {
    return "Preferential Attachment";
}

std::string PreferentialAttachmentPredictor::getDescription() const {
    return "Predicts links using igraph-based node degrees: score = degree(u) x degree(v)";
}

} // namespace qlink