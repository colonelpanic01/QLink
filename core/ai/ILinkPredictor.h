#pragma once

#include <QObject>
#include <vector>
#include <memory>
#include <string>
#include <map>
#include "../common/DataStructures.h"
#include <igraph/igraph.h>

namespace qlink {

class MentalModel;

/**
 * Interface for link prediction algorithms (Strategy Pattern)
 */
class ILinkPredictor : public QObject {
    Q_OBJECT

public:
    explicit ILinkPredictor(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~ILinkPredictor() = default;
    
    /**
     * Predict potential links in the given mental model
     * @param model The mental model to analyze
     * @param maxSuggestions Maximum number of suggestions to return
     * @return Vector of link suggestions ranked by confidence
     */
    virtual std::vector<LinkSuggestion> predictLinks(const MentalModel& model, int maxSuggestions = 10) = 0;
    
    /**
     * Get the name of this prediction algorithm
     * @return Algorithm name for display purposes
     */
    virtual std::string getAlgorithmName() const = 0;
    
    /**
     * Get a description of how this algorithm works
     * @return Algorithm description
     */
    virtual std::string getDescription() const = 0;
};

/**
 * Base class for igraph-based link predictors
 * Provides common functionality for converting MentalModel to igraph format
 */
class IGraphLinkPredictor : public ILinkPredictor {
    Q_OBJECT

protected:
    explicit IGraphLinkPredictor(QObject *parent = nullptr) : ILinkPredictor(parent) {}
    
    /**
     * Convert MentalModel to igraph structure
     */
    void convertToIGraph(const MentalModel& model, igraph_t* graph, std::map<std::string, int>& conceptToVertex);
    
    /**
     * Convert igraph similarity matrix to LinkSuggestions
     */
    std::vector<LinkSuggestion> convertSimilarityToSuggestions(
        const igraph_matrix_t* similarity,
        const std::map<std::string, int>& conceptToVertex,
        const MentalModel& model,
        int maxSuggestions,
        const std::string& algorithmName);
        
    /**
     * Clean up igraph resources
     */
    void cleanupIGraph(igraph_t* graph);
};

/**
 * Factory for creating link predictor instances
 */
class LinkPredictorFactory {
public:
    enum class AlgorithmType {
        COMMON_NEIGHBORS,
        JACCARD_COEFFICIENT,
        PREFERENTIAL_ATTACHMENT
    };
    
    static std::unique_ptr<ILinkPredictor> createPredictor(AlgorithmType type);
    static std::vector<AlgorithmType> getAvailableAlgorithms();
    static std::string getAlgorithmName(AlgorithmType type);
};

} // namespace qlink