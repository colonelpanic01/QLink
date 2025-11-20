#include "ILinkPredictor.h"
#include "CommonNeighborPredictor.h"
#include "JaccardCoefficientPredictor.h"
#include "PreferentialAttachmentPredictor.h"
#include <stdexcept>

namespace qlink {

std::unique_ptr<ILinkPredictor> LinkPredictorFactory::createPredictor(LinkPredictorFactory::AlgorithmType type) {
    switch (type) {
        case LinkPredictorFactory::AlgorithmType::COMMON_NEIGHBORS:
            return std::make_unique<CommonNeighborPredictor>();
        case LinkPredictorFactory::AlgorithmType::JACCARD_COEFFICIENT:
            return std::make_unique<JaccardCoefficientPredictor>();
        case LinkPredictorFactory::AlgorithmType::PREFERENTIAL_ATTACHMENT:
            return std::make_unique<PreferentialAttachmentPredictor>();
        default:
            throw std::runtime_error("Unknown algorithm type");
    }
}

std::vector<LinkPredictorFactory::AlgorithmType> LinkPredictorFactory::getAvailableAlgorithms() {
    return {
        LinkPredictorFactory::AlgorithmType::COMMON_NEIGHBORS,
        LinkPredictorFactory::AlgorithmType::JACCARD_COEFFICIENT,
        LinkPredictorFactory::AlgorithmType::PREFERENTIAL_ATTACHMENT
    };
}

std::string LinkPredictorFactory::getAlgorithmName(LinkPredictorFactory::AlgorithmType type) {
    switch (type) {
        case LinkPredictorFactory::AlgorithmType::COMMON_NEIGHBORS:
            return "Common Neighbors";
        case LinkPredictorFactory::AlgorithmType::JACCARD_COEFFICIENT:
            return "Jaccard Coefficient";
        case LinkPredictorFactory::AlgorithmType::PREFERENTIAL_ATTACHMENT:
            return "Preferential Attachment";
        default:
            return "Unknown Algorithm";
    }
}

} // namespace qlink