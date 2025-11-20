#pragma once

#include "ILinkPredictor.h"
#include "../model/MentalModel.h"
#include <vector>
#include <string>
#include <set>

namespace qlink {

/**
 * Link predictor using Jaccard Coefficient algorithm
 * Jaccard coefficient = |N(u) ∩ N(v)| / |N(u) ∪ N(v)|
 * where N(u) is the set of neighbors of node u
 */
class JaccardCoefficientPredictor : public ILinkPredictor {
    Q_OBJECT

public:
    explicit JaccardCoefficientPredictor(QObject *parent = nullptr);
    ~JaccardCoefficientPredictor() = default;

    // Inherited from ILinkPredictor
    std::vector<LinkSuggestion> predictLinks(const MentalModel& model, 
                                            int maxSuggestions = 10) override;
    
    std::string getAlgorithmName() const override;
    std::string getDescription() const override;

private:

private:
    /**
     * Calculate Jaccard coefficient between two concepts
     * @param model The mental model containing the graph
     * @param concept1Id ID of first concept
     * @param concept2Id ID of second concept
     * @return Jaccard coefficient (0.0 to 1.0)
     */
    double calculateJaccardCoefficient(const MentalModel& model, 
                                     const std::string& concept1Id, 
                                     const std::string& concept2Id);

    /**
     * Get all neighbors of a concept
     * @param model The mental model
     * @param conceptId ID of the concept
     * @return Set of neighbor concept IDs
     */
    std::set<std::string> getNeighbors(const MentalModel& model, const std::string& conceptId);

    // Note: Using standard library set operations instead of custom methods
};

} // namespace qlink