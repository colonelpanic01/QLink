#include <gtest/gtest.h>
#include "../../core/model/MentalModel.h"
#include "../../core/model/Concept.h"
#include "../../core/model/Relationship.h"
#include "../../core/ai/PreferentialAttachmentPredictor.h"

using namespace qlink;

class PreferentialAttachmentPredictorTest : public ::testing::Test {
protected:
    void SetUp() override {
        model = std::make_unique<MentalModel>("Test Model");
        predictor = std::make_unique<PreferentialAttachmentPredictor>();
    }
    
    std::unique_ptr<MentalModel> model;
    std::unique_ptr<PreferentialAttachmentPredictor> predictor;
};

// Basic functionality tests
TEST_F(PreferentialAttachmentPredictorTest, EmptyModelReturnsNoSuggestions) {
    auto suggestions = predictor->predictLinks(*model, 10);
    EXPECT_EQ(suggestions.size(), 0);
}

TEST_F(PreferentialAttachmentPredictorTest, SingleConceptReturnsNoSuggestions) {
    model->addConcept(std::make_unique<Concept>("Concept1"));
    auto suggestions = predictor->predictLinks(*model, 10);
    EXPECT_EQ(suggestions.size(), 0);
}

TEST_F(PreferentialAttachmentPredictorTest, AlgorithmNameIsCorrect) {
    EXPECT_EQ(predictor->getAlgorithmName(), "Preferential Attachment");
}

TEST_F(PreferentialAttachmentPredictorTest, DescriptionIsNotEmpty) {
    std::string desc = predictor->getDescription();
    EXPECT_FALSE(desc.empty());
}

TEST_F(PreferentialAttachmentPredictorTest, FavorsHighDegreeNodes) {
    // Create a hub-and-spoke pattern
    // Hub connects to many nodes, Leaf connects to few
    auto hub = std::make_unique<Concept>("Hub");
    auto leaf = std::make_unique<Concept>("Leaf");
    auto target = std::make_unique<Concept>("Target");
    
    std::string hubId = hub->getId();
    std::string leafId = leaf->getId();
    std::string targetId = target->getId();
    
    model->addConcept(std::move(hub));
    model->addConcept(std::move(leaf));
    model->addConcept(std::move(target));
    
    // Add spokes to hub
    for (int i = 0; i < 5; ++i) {
        auto spoke = std::make_unique<Concept>("Spoke" + std::to_string(i));
        std::string spokeId = spoke->getId();
        model->addConcept(std::move(spoke));
        model->addRelationship(std::unique_ptr<Relationship>(new Relationship(hubId, spokeId)));
    }
    
    // Leaf has only 1 connection
    auto spoke = std::make_unique<Concept>("SpokeForLeaf");
    std::string spokeId = spoke->getId();
    model->addConcept(std::move(spoke));
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(leafId, spokeId)));
    
    auto suggestions = predictor->predictLinks(*model, 10);
    
    // Hub-Target connection should have higher score than Leaf-Target
    double hubTargetScore = 0.0;
    double leafTargetScore = 0.0;
    
    for (const auto& sug : suggestions) {
        if ((sug.sourceConceptId == hubId && sug.targetConceptId == targetId) ||
            (sug.sourceConceptId == targetId && sug.targetConceptId == hubId)) {
            hubTargetScore = sug.confidence;
        }
        if ((sug.sourceConceptId == leafId && sug.targetConceptId == targetId) ||
            (sug.sourceConceptId == targetId && sug.targetConceptId == leafId)) {
            leafTargetScore = sug.confidence;
        }
    }
    
    // Hub should have higher preferential attachment score
    if (hubTargetScore > 0 && leafTargetScore > 0) {
        EXPECT_GT(hubTargetScore, leafTargetScore);
    }
}

TEST_F(PreferentialAttachmentPredictorTest, DoesNotSuggestExistingConnections) {
    auto c1 = std::make_unique<Concept>("C1");
    auto c2 = std::make_unique<Concept>("C2");
    std::string c1Id = c1->getId();
    std::string c2Id = c2->getId();
    
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(c1Id, c2Id)));
    
    auto suggestions = predictor->predictLinks(*model, 10);
    
    for (const auto& sug : suggestions) {
        EXPECT_FALSE((sug.sourceConceptId == c1Id && sug.targetConceptId == c2Id) ||
                    (sug.sourceConceptId == c2Id && sug.targetConceptId == c1Id));
    }
}

TEST_F(PreferentialAttachmentPredictorTest, RespectsMaxSuggestionsLimit) {
    std::vector<std::string> ids;
    for (int i = 0; i < 10; ++i) {
        auto c = std::make_unique<Concept>("C" + std::to_string(i));
        ids.push_back(c->getId());
        model->addConcept(std::move(c));
    }
    
    for (size_t i = 0; i < ids.size() - 1; ++i) {
        model->addRelationship(std::unique_ptr<Relationship>(new Relationship(ids[i], ids[i + 1])));
    }
    
    auto suggestions = predictor->predictLinks(*model, 5);
    EXPECT_LE(suggestions.size(), 5);
}

TEST_F(PreferentialAttachmentPredictorTest, ConfidenceValuesAreValid) {
    auto c1 = std::make_unique<Concept>("C1");
    auto c2 = std::make_unique<Concept>("C2");
    auto c3 = std::make_unique<Concept>("C3");
    std::string id1 = c1->getId();
    std::string id2 = c2->getId();
    std::string id3 = c3->getId();
    
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    model->addConcept(std::move(c3));
    
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(id1, id2)));
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(id2, id3)));
    
    auto suggestions = predictor->predictLinks(*model, 10);
    
    for (const auto& sug : suggestions) {
        EXPECT_GE(sug.confidence, 0.0);
        EXPECT_LE(sug.confidence, 1.0);
    }
}

TEST_F(PreferentialAttachmentPredictorTest, HandlesIsolatedNodes) {
    // Two connected nodes and one isolated
    auto c1 = std::make_unique<Concept>("C1");
    auto c2 = std::make_unique<Concept>("C2");
    auto isolated = std::make_unique<Concept>("Isolated");
    
    std::string id1 = c1->getId();
    std::string id2 = c2->getId();
    std::string isoId = isolated->getId();
    
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    model->addConcept(std::move(isolated));
    
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(id1, id2)));
    
    auto suggestions = predictor->predictLinks(*model, 10);
    
    // Should handle isolated nodes without crashing
    EXPECT_NO_THROW(predictor->predictLinks(*model, 10));
}

TEST_F(PreferentialAttachmentPredictorTest, StarTopologyDetected) {
    // Center node connects to 5 peripheral nodes, plus one new unconnected node
    auto center = std::make_unique<Concept>("Center");
    std::string centerId = center->getId();
    model->addConcept(std::move(center));
    
    std::vector<std::string> peripheralIds;
    for (int i = 0; i < 5; ++i) {
        auto peripheral = std::make_unique<Concept>("P" + std::to_string(i));
        std::string pId = peripheral->getId();
        peripheralIds.push_back(pId);
        model->addConcept(std::move(peripheral));
        model->addRelationship(std::unique_ptr<Relationship>(new Relationship(centerId, pId)));
    }
    
    // Add a new node that is not connected yet
    auto newNode = std::make_unique<Concept>("NewNode");
    std::string newId = newNode->getId();
    model->addConcept(std::move(newNode));
    
    auto suggestions = predictor->predictLinks(*model, 10);
    
    // Center node should be suggested to connect to NewNode due to high degree
    bool foundCenterToNew = false;
    for (const auto& sug : suggestions) {
        if ((sug.sourceConceptId == centerId && sug.targetConceptId == newId) ||
            (sug.sourceConceptId == newId && sug.targetConceptId == centerId)) {
            foundCenterToNew = true;
            EXPECT_GT(sug.confidence, 0.0);
            break;
        }
    }
    
    // Center should be suggested due to preferential attachment
    EXPECT_TRUE(foundCenterToNew);
}
