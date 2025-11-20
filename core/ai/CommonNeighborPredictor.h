#pragma once

#include "ILinkPredictor.h"
#include "../model/MentalModel.h"
#include <vector>
#include <string>

namespace qlink {

/**
 * Common Neighbors algorithm for link prediction where the
 * score = number of common neighbors between two concepts
 */
class CommonNeighborPredictor : public ILinkPredictor {
    Q_OBJECT

public:
    explicit CommonNeighborPredictor(QObject *parent = nullptr);
    ~CommonNeighborPredictor() = default;

    std::vector<LinkSuggestion> predictLinks(const MentalModel& model, int maxSuggestions = 10) override;
    std::string getAlgorithmName() const override { return "Common Neighbors"; }
    std::string getDescription() const override { 
        return "Predicts links based on the number of common neighbors between concepts"; 
    }

private:
    /**
     * Calculate common neighbor score between two concepts
     * @param model The mental model
     * @param concept1Id ID of first concept
     * @param concept2Id ID of second concept
     * @return Number of common neighbors
     */
    int calculateCommonNeighbors(const MentalModel& model,
                                const std::string& concept1Id,
                                const std::string& concept2Id);
    
    /**
     * Get neighbors of a concept
     * @param model The mental model
     * @param conceptId ID of the concept
     * @return Set of neighbor IDs
     */
    std::vector<std::string> getNeighbors(const MentalModel& model, 
                                         const std::string& conceptId);
};

} // namespace qlink