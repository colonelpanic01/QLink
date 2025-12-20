#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../core/model/MentalModel.h"
#include "../../core/model/Concept.h"

using namespace qlink;

// TODO: Implement MentalModel tests in next deliverable
// These are placeholder tests

class MentalModelTest : public ::testing::Test {
protected:
    void SetUp() override {
        model = std::make_unique<MentalModel>("Test Model");
    }
    
    std::unique_ptr<MentalModel> model;
};

TEST_F(MentalModelTest, InitialStateIsEmpty) {
    EXPECT_TRUE(model->isEmpty());
    EXPECT_EQ(model->getConceptCount(), 0);
    EXPECT_EQ(model->getRelationshipCount(), 0);
}

TEST_F(MentalModelTest, CanAddConcept) {
    auto concept = std::make_unique<Concept>("Test Concept");
    std::string conceptId = concept->getId();
    
    model->addConcept(std::move(concept));
    
    EXPECT_EQ(model->getConceptCount(), 1);
    EXPECT_FALSE(model->isEmpty());
    EXPECT_NE(model->getConcept(conceptId), nullptr);
}
