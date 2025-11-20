#pragma once

#include <QObject>
#include <vector>
#include <memory>
#include <string>
#include "../common/DataStructures.h"

namespace qlink {

// Forward declaration
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