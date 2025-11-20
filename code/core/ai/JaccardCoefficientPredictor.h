#pragma once

#include "ILinkPredictor.h"
#include "../model/MentalModel.h"
#include <vector>
#include <string>
#include <set>

namespace qlink {

/**
 * Link predictor using Jaccard Coefficient algorithm with igraph
 */
class JaccardCoefficientPredictor : public IGraphLinkPredictor {
    Q_OBJECT

public:
    explicit JaccardCoefficientPredictor(QObject *parent = nullptr);
    ~JaccardCoefficientPredictor() = default;

    // Inherited from ILinkPredictor
    std::vector<LinkSuggestion> predictLinks(const MentalModel& model, 
                                            int maxSuggestions = 10) override;
    
    std::string getAlgorithmName() const override;
    std::string getDescription() const override;

};

} // namespace qlink