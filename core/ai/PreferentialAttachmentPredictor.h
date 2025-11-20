#pragma once

#include "ILinkPredictor.h"
#include "../model/MentalModel.h"
#include <vector>
#include <string>
#include <map>

namespace qlink {

/**
 * Link predictor using Preferential Attachment algorithm
 * Preferential Attachment score = degree(u) * degree(v)
 * This algorithm favors connections between high-degree nodes
 */
class PreferentialAttachmentPredictor : public ILinkPredictor {
    Q_OBJECT

public:
    explicit PreferentialAttachmentPredictor(QObject *parent = nullptr);
    ~PreferentialAttachmentPredictor() = default;

    // Inherited from ILinkPredictor
    std::vector<LinkSuggestion> predictLinks(const MentalModel& model, 
                                            int maxSuggestions = 10) override;
    
    std::string getAlgorithmName() const override;
    std::string getDescription() const override;

private:

private:
    /**
     * Calculate preferential attachment score between two concepts
     * @param model The mental model containing the graph
     * @param concept1Id ID of first concept
     * @param concept2Id ID of second concept
     * @return Preferential attachment score (normalized between 0 and 1)
     */
    double calculatePreferentialAttachmentScore(const MentalModel& model, 
                                              const std::string& concept1Id, 
                                              const std::string& concept2Id);

    /**
     * Get the degree (number of connections) of a concept
     * @param model The mental model
     * @param conceptId ID of the concept
     * @return Degree of the concept
     */
    int getDegree(const MentalModel& model, const std::string& conceptId);

    /**
     * Calculate degree distribution statistics for normalization
     * @param model The mental model
     * @return Map of concept ID to degree
     */
    std::map<std::string, int> calculateDegreeDistribution(const MentalModel& model);

    /**
     * Normalize score based on degree distribution
     * @param rawScore Raw preferential attachment score
     * @param maxPossibleScore Maximum possible score in current graph
     * @return Normalized score between 0 and 1
     */
    double normalizeScore(double rawScore, double maxPossibleScore);
};

} // namespace qlink