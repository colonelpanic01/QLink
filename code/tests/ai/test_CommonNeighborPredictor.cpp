#include <gtest/gtest.h>
#include "../../core/model/MentalModel.h"
#include "../../core/model/Concept.h"
#include "../../core/model/Relationship.h"
#include "../../core/ai/CommonNeighborPredictor.h"

using namespace qlink;

class CommonNeighborPredictorTest : public ::testing::Test {
protected:
    void SetUp() override {
        model = std::make_unique<MentalModel>("Test Model");
        predictor = std::make_unique<CommonNeighborPredictor>();
    }
    
    std::unique_ptr<MentalModel> model;
    std::unique_ptr<CommonNeighborPredictor> predictor;
};

// Basic functionality tests
TEST_F(CommonNeighborPredictorTest, EmptyModelReturnsNoSuggestions) {
    auto suggestions = predictor->predictLinks(*model, 10);
    EXPECT_EQ(suggestions.size(), 0);
}

TEST_F(CommonNeighborPredictorTest, SingleConceptReturnsNoSuggestions) {
    model->addConcept(std::make_unique<Concept>("Concept1"));
    auto suggestions = predictor->predictLinks(*model, 10);
    EXPECT_EQ(suggestions.size(), 0);
}

TEST_F(CommonNeighborPredictorTest, TwoUnconnectedConceptsReturnNoSuggestions) {
    model->addConcept(std::make_unique<Concept>("C1"));
    model->addConcept(std::make_unique<Concept>("C2"));
    auto suggestions = predictor->predictLinks(*model, 10);
    EXPECT_EQ(suggestions.size(), 0);
}

TEST_F(CommonNeighborPredictorTest, TrianglePatternDetected) {
    // Create a triangle: A-B, B-C, should suggest A-C
    auto c1 = std::make_unique<Concept>("A");
    auto c2 = std::make_unique<Concept>("B");
    auto c3 = std::make_unique<Concept>("C");
    std::string c1Id = c1->getId();
    std::string c2Id = c2->getId();
    std::string c3Id = c3->getId();
    
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    model->addConcept(std::move(c3));
    
    // Connect A-B and B-C (B is common neighbor)
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(c1Id, c2Id)));
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(c2Id, c3Id)));
    
    auto suggestions = predictor->predictLinks(*model, 10);
    
    // Should suggest A-C connection (they share common neighbor B)
    EXPECT_GT(suggestions.size(), 0);
    bool foundConnection = false;
    for (const auto& sug : suggestions) {
        if ((sug.sourceConceptId == c1Id && sug.targetConceptId == c3Id) ||
            (sug.sourceConceptId == c3Id && sug.targetConceptId == c1Id)) {
            foundConnection = true;
            EXPECT_GT(sug.confidence, 0.0);
            EXPECT_EQ(sug.algorithmName, "Common Neighbors");
            break;
        }
    }
    EXPECT_TRUE(foundConnection);
}

TEST_F(CommonNeighborPredictorTest, MultipleCommonNeighborsIncreaseConfidence) {
    // Create star pattern: A and B both connect to C, D, E
    auto cA = std::make_unique<Concept>("A");
    auto cB = std::make_unique<Concept>("B");
    auto cC = std::make_unique<Concept>("C");
    auto cD = std::make_unique<Concept>("D");
    auto cE = std::make_unique<Concept>("E");
    
    std::string idA = cA->getId();
    std::string idB = cB->getId();
    std::string idC = cC->getId();
    std::string idD = cD->getId();
    std::string idE = cE->getId();
    
    model->addConcept(std::move(cA));
    model->addConcept(std::move(cB));
    model->addConcept(std::move(cC));
    model->addConcept(std::move(cD));
    model->addConcept(std::move(cE));
    
    // A connects to C, D, E
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(idA, idC)));
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(idA, idD)));
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(idA, idE)));
    
    // B connects to C, D, E (same neighbors as A)
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(idB, idC)));
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(idB, idD)));
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(idB, idE)));
    
    auto suggestions = predictor->predictLinks(*model, 10);
    
    // Should suggest A-B with high confidence (3 common neighbors)
    bool foundAB = false;
    for (const auto& sug : suggestions) {
        if ((sug.sourceConceptId == idA && sug.targetConceptId == idB) ||
            (sug.sourceConceptId == idB && sug.targetConceptId == idA)) {
            foundAB = true;
            EXPECT_GT(sug.confidence, 0.5); // High confidence due to 3 common neighbors
            break;
        }
    }
    EXPECT_TRUE(foundAB);
}

TEST_F(CommonNeighborPredictorTest, DoesNotSuggestExistingConnections) {
    auto c1 = std::make_unique<Concept>("C1");
    auto c2 = std::make_unique<Concept>("C2");
    std::string c1Id = c1->getId();
    std::string c2Id = c2->getId();
    
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(c1Id, c2Id)));
    
    auto suggestions = predictor->predictLinks(*model, 10);
    
    // Should not suggest already existing connection
    for (const auto& sug : suggestions) {
        EXPECT_FALSE((sug.sourceConceptId == c1Id && sug.targetConceptId == c2Id) ||
                    (sug.sourceConceptId == c2Id && sug.targetConceptId == c1Id));
    }
}

TEST_F(CommonNeighborPredictorTest, RespectsMaxSuggestionsLimit) {
    // Create a large graph
    std::vector<std::string> ids;
    for (int i = 0; i < 10; ++i) {
        auto c = std::make_unique<Concept>("C" + std::to_string(i));
        ids.push_back(c->getId());
        model->addConcept(std::move(c));
    }
    
    // Create connections to generate many potential suggestions
    for (size_t i = 0; i < ids.size() - 1; ++i) {
        model->addRelationship(std::unique_ptr<Relationship>(new Relationship(ids[i], ids[i + 1])));
    }
    
    auto suggestions = predictor->predictLinks(*model, 5);
    EXPECT_LE(suggestions.size(), 5);
}

TEST_F(CommonNeighborPredictorTest, AlgorithmNameIsCorrect) {
    EXPECT_EQ(predictor->getAlgorithmName(), "Common Neighbors");
}

TEST_F(CommonNeighborPredictorTest, DescriptionIsNotEmpty) {
    std::string desc = predictor->getDescription();
    EXPECT_FALSE(desc.empty());
    EXPECT_GT(desc.length(), 10);
}

TEST_F(CommonNeighborPredictorTest, DisconnectedComponentsHandled) {
    // Create two separate triangles
    auto c1 = std::make_unique<Concept>("C1");
    auto c2 = std::make_unique<Concept>("C2");
    auto c3 = std::make_unique<Concept>("C3");
    auto c4 = std::make_unique<Concept>("C4");
    auto c5 = std::make_unique<Concept>("C5");
    auto c6 = std::make_unique<Concept>("C6");
    
    std::string id1 = c1->getId();
    std::string id2 = c2->getId();
    std::string id3 = c3->getId();
    std::string id4 = c4->getId();
    std::string id5 = c5->getId();
    std::string id6 = c6->getId();
    
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    model->addConcept(std::move(c3));
    model->addConcept(std::move(c4));
    model->addConcept(std::move(c5));
    model->addConcept(std::move(c6));
    
    // Triangle 1: C1-C2, C2-C3
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(id1, id2)));
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(id2, id3)));
    
    // Triangle 2: C4-C5, C5-C6
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(id4, id5)));
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(id5, id6)));
    
    auto suggestions = predictor->predictLinks(*model, 10);
    
    // Should not crash and should return suggestions within each component
    EXPECT_NO_THROW(predictor->predictLinks(*model, 10));
}

TEST_F(CommonNeighborPredictorTest, ConfidenceValuesAreNormalized) {
    auto c1 = std::make_unique<Concept>("C1");
    auto c2 = std::make_unique<Concept>("C2");
    auto c3 = std::make_unique<Concept>("C3");
    std::string id1 = c1->getId();
    std::string id2 = c2->getId();
    std::string id3 = c3->getId();
    
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    model->addConcept(std::move(c3));
    
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(id1, id3)));
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(id2, id3)));
    
    auto suggestions = predictor->predictLinks(*model, 10);
    
    for (const auto& sug : suggestions) {
        EXPECT_GE(sug.confidence, 0.0);
        EXPECT_LE(sug.confidence, 1.0);
    }
}
