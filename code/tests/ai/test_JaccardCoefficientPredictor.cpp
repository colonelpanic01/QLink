#include <gtest/gtest.h>
#include "../../core/model/MentalModel.h"
#include "../../core/model/Concept.h"
#include "../../core/model/Relationship.h"
#include "../../core/ai/JaccardCoefficientPredictor.h"

using namespace qlink;

class JaccardCoefficientPredictorTest : public ::testing::Test {
protected:
    void SetUp() override {
        model = std::make_unique<MentalModel>("Test Model");
        predictor = std::make_unique<JaccardCoefficientPredictor>();
    }
    
    std::unique_ptr<MentalModel> model;
    std::unique_ptr<JaccardCoefficientPredictor> predictor;
};

// Basic functionality tests
TEST_F(JaccardCoefficientPredictorTest, EmptyModelReturnsNoSuggestions) {
    auto suggestions = predictor->predictLinks(*model, 10);
    EXPECT_EQ(suggestions.size(), 0);
}

TEST_F(JaccardCoefficientPredictorTest, SingleConceptReturnsNoSuggestions) {
    model->addConcept(std::make_unique<Concept>("Concept1"));
    auto suggestions = predictor->predictLinks(*model, 10);
    EXPECT_EQ(suggestions.size(), 0);
}

TEST_F(JaccardCoefficientPredictorTest, AlgorithmNameIsCorrect) {
    EXPECT_EQ(predictor->getAlgorithmName(), "Jaccard Coefficient");
}

TEST_F(JaccardCoefficientPredictorTest, DescriptionIsNotEmpty) {
    std::string desc = predictor->getDescription();
    EXPECT_FALSE(desc.empty());
}

TEST_F(JaccardCoefficientPredictorTest, PredictsSimilarNeighborhoods) {
    // Create scenario where A and B have similar but not identical neighbors
    auto cA = std::make_unique<Concept>("A");
    auto cB = std::make_unique<Concept>("B");
    auto c1 = std::make_unique<Concept>("C1");
    auto c2 = std::make_unique<Concept>("C2");
    auto c3 = std::make_unique<Concept>("C3");
    
    std::string idA = cA->getId();
    std::string idB = cB->getId();
    std::string id1 = c1->getId();
    std::string id2 = c2->getId();
    std::string id3 = c3->getId();
    
    model->addConcept(std::move(cA));
    model->addConcept(std::move(cB));
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    model->addConcept(std::move(c3));
    
    // A connects to C1, C2
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(idA, id1)));
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(idA, id2)));
    
    // B connects to C1, C2, C3 (shares 2 out of 3 neighbors with A)
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(idB, id1)));
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(idB, id2)));
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(idB, id3)));
    
    auto suggestions = predictor->predictLinks(*model, 10);
    
    // Should suggest A-B based on Jaccard similarity
    bool foundAB = false;
    for (const auto& sug : suggestions) {
        if ((sug.sourceConceptId == idA && sug.targetConceptId == idB) ||
            (sug.sourceConceptId == idB && sug.targetConceptId == idA)) {
            foundAB = true;
            EXPECT_GT(sug.confidence, 0.0);
            EXPECT_EQ(sug.algorithmName, "Jaccard Coefficient");
            break;
        }
    }
    EXPECT_TRUE(foundAB);
}

TEST_F(JaccardCoefficientPredictorTest, DoesNotSuggestExistingConnections) {
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

TEST_F(JaccardCoefficientPredictorTest, RespectsMaxSuggestionsLimit) {
    std::vector<std::string> ids;
    for (int i = 0; i < 10; ++i) {
        auto c = std::make_unique<Concept>("C" + std::to_string(i));
        ids.push_back(c->getId());
        model->addConcept(std::move(c));
    }
    
    for (size_t i = 0; i < ids.size() - 1; ++i) {
        model->addRelationship(std::unique_ptr<Relationship>(new Relationship(ids[i], ids[i + 1])));
    }
    
    auto suggestions = predictor->predictLinks(*model, 3);
    EXPECT_LE(suggestions.size(), 3);
}

TEST_F(JaccardCoefficientPredictorTest, ConfidenceValuesAreValid) {
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

TEST_F(JaccardCoefficientPredictorTest, HandlesNoCommonNeighbors) {
    // A connects to C1, B connects to C2 (no overlap)
    auto cA = std::make_unique<Concept>("A");
    auto cB = std::make_unique<Concept>("B");
    auto c1 = std::make_unique<Concept>("C1");
    auto c2 = std::make_unique<Concept>("C2");
    
    std::string idA = cA->getId();
    std::string idB = cB->getId();
    std::string id1 = c1->getId();
    std::string id2 = c2->getId();
    
    model->addConcept(std::move(cA));
    model->addConcept(std::move(cB));
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(idA, id1)));
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(idB, id2)));
    
    auto suggestions = predictor->predictLinks(*model, 10);
    
    // Should not suggest A-B (Jaccard coefficient = 0)
    for (const auto& sug : suggestions) {
        if ((sug.sourceConceptId == idA && sug.targetConceptId == idB) ||
            (sug.sourceConceptId == idB && sug.targetConceptId == idA)) {
            FAIL() << "Should not suggest connection with zero Jaccard coefficient";
        }
    }
}
