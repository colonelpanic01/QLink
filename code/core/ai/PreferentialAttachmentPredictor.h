#pragma once

#include "ILinkPredictor.h"
#include "../model/MentalModel.h"
#include <vector>
#include <string>
#include <map>

namespace qlink {

/**
 * Link predictor using Preferential Attachment algorithm with igraph
 * Preferential Attachment score = degree(u) * degree(v)
 * This algorithm favors connections between high-degree nodes
 */
class PreferentialAttachmentPredictor : public IGraphLinkPredictor {
    Q_OBJECT

public:
    explicit PreferentialAttachmentPredictor(QObject *parent = nullptr);
    ~PreferentialAttachmentPredictor() = default;

    // Inherited from ILinkPredictor
    std::vector<LinkSuggestion> predictLinks(const MentalModel& model, 
                                            int maxSuggestions = 10) override;
    
    std::string getAlgorithmName() const override;
    std::string getDescription() const override;

};

} // namespace qlink