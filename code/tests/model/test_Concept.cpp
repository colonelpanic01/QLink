#include <gtest/gtest.h>
#include "../../core/model/Concept.h"

using namespace qlink;

// TODO: Implement Concept tests in next deliverable
// These are placeholder tests

class ConceptTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code for each test
    }
    
    void TearDown() override {
        // Cleanup code for each test
    }
};

TEST_F(ConceptTest, ConstructorSetsNameCorrectly) {
    // Use the constructor that takes name and description
    Concept concept("Energy", "The capacity to do work");
    EXPECT_EQ(concept.getName(), "Energy");
    EXPECT_EQ(concept.getDescription(), "The capacity to do work");
}

TEST_F(ConceptTest, IdIsUniqueForEachConcept) {
    // Use the constructor that takes name and description (empty description)
    Concept concept1("Concept1", "");
    Concept concept2("Concept2", "");
    EXPECT_NE(concept1.getId(), concept2.getId());
}
