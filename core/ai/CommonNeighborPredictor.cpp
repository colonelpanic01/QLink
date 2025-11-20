#include "CommonNeighborPredictor.h"
#include "../model/MentalModel.h"

namespace qlink {

CommonNeighborPredictor::CommonNeighborPredictor(QObject *parent)
    : IGraphLinkPredictor(parent) {
}

std::vector<LinkSuggestion> CommonNeighborPredictor::predictLinks(const MentalModel& model, int maxSuggestions) {
    const auto& concepts = model.getConcepts();
    
    if (concepts.size() < 2) {
        return std::vector<LinkSuggestion>();
    }
    
    // Convert to igraph
    igraph_t graph;
    std::map<std::string, int> conceptToVertex;
    convertToIGraph(model, &graph, conceptToVertex);
    
    // Calculate similarity using igraph
    igraph_matrix_t similarity;
    igraph_matrix_init(&similarity, 0, 0);
    
    // Use igraph's common neighbors calculation
    // Note: igraph doesn't have direct common neighbors, so using a slightly custom approach
    int numVertices = igraph_vcount(&graph);
    igraph_matrix_resize(&similarity, numVertices, numVertices);
    igraph_matrix_fill(&similarity, 0.0);
    
    // Calculate common neighbors for each pair
    for (int i = 0; i < numVertices; ++i) {
        for (int j = i + 1; j < numVertices; ++j) {
            igraph_vector_int_t neighbors_i, neighbors_j;
            igraph_vector_int_init(&neighbors_i, 0);
            igraph_vector_int_init(&neighbors_j, 0);
            
            // Get neighbors of both vertices (updated for igraph 1.0+)
            igraph_neighbors(&graph, &neighbors_i, i, IGRAPH_ALL, IGRAPH_NO_LOOPS, IGRAPH_NO_MULTIPLE);
            igraph_neighbors(&graph, &neighbors_j, j, IGRAPH_ALL, IGRAPH_NO_LOOPS, IGRAPH_NO_MULTIPLE);
            
            // Count common neighbors
            int commonCount = 0;
            for (int ni = 0; ni < igraph_vector_int_size(&neighbors_i); ++ni) {
                int neighbor_i = VECTOR(neighbors_i)[ni];
                for (int nj = 0; nj < igraph_vector_int_size(&neighbors_j); ++nj) {
                    int neighbor_j = VECTOR(neighbors_j)[nj];
                    if (neighbor_i == neighbor_j) {
                        commonCount++;
                        break;
                    }
                }
            }
            
            // Store raw common count as score
            double score = static_cast<double>(commonCount);
            MATRIX(similarity, i, j) = score;
            MATRIX(similarity, j, i) = score;
            
            igraph_vector_int_destroy(&neighbors_i);
            igraph_vector_int_destroy(&neighbors_j);
        }
    }
    
    // Convert to suggestions
    auto suggestions = convertSimilarityToSuggestions(&similarity, conceptToVertex, model, maxSuggestions, "Common Neighbors");
    
    // Cleanup
    igraph_matrix_destroy(&similarity);
    cleanupIGraph(&graph);
    
    return suggestions;
}

} // namespace qlink

