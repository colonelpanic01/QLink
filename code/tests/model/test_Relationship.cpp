#include <gtest/gtest.h>
#include "../../core/model/Relationship.h"

using namespace qlink;

// TODO: Implement Relationship tests in next deliverable
// These are placeholder tests

class RelationshipTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code for each test
    }
};

TEST_F(RelationshipTest, ConstructorSetsPropertiesCorrectly) {
    Relationship rel("concept1", "concept2", "causes", true, 0.8);
    EXPECT_EQ(rel.getSourceConceptId(), "concept1");
    EXPECT_EQ(rel.getTargetConceptId(), "concept2");
    EXPECT_EQ(rel.getType(), "causes");
    EXPECT_TRUE(rel.getIsDirected());
    EXPECT_DOUBLE_EQ(rel.getWeight(), 0.8);
}
