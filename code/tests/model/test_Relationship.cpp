#include <gtest/gtest.h>
#include "../../core/model/Relationship.h"

using namespace qlink;

class RelationshipTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code for each test
    }
};

// Constructor tests
TEST_F(RelationshipTest, ConstructorSetsPropertiesCorrectly) {
    Relationship rel("concept1", "concept2", "causes", true, 0.8);
    EXPECT_EQ(rel.getSourceConceptId(), "concept1");
    EXPECT_EQ(rel.getTargetConceptId(), "concept2");
    EXPECT_EQ(rel.getType(), "causes");
    EXPECT_TRUE(rel.getIsDirected());
    EXPECT_DOUBLE_EQ(rel.getWeight(), 0.8);
}

TEST_F(RelationshipTest, ConstructorWithDefaults) {
    Relationship rel("source", "target", "", false, 1.0);
    EXPECT_EQ(rel.getSourceConceptId(), "source");
    EXPECT_EQ(rel.getTargetConceptId(), "target");
    EXPECT_EQ(rel.getType(), "");
    EXPECT_FALSE(rel.getIsDirected());
    EXPECT_DOUBLE_EQ(rel.getWeight(), 1.0);
}

TEST_F(RelationshipTest, ConstructorWithCustomId) {
    Relationship rel("custom-id", "source", "target", "type", true, 0.5);
    EXPECT_EQ(rel.getId(), "custom-id");
    EXPECT_EQ(rel.getSourceConceptId(), "source");
    EXPECT_EQ(rel.getTargetConceptId(), "target");
}

TEST_F(RelationshipTest, IdIsNotEmpty) {
    Relationship rel("source", "target", "", false, 1.0);
    EXPECT_FALSE(rel.getId().empty());
}

TEST_F(RelationshipTest, IdIsUniqueForEachRelationship) {
    Relationship rel1("source", "target", "", false, 1.0);
    Relationship rel2("source", "target", "", false, 1.0);
    EXPECT_NE(rel1.getId(), rel2.getId());
}

// Setter tests
TEST_F(RelationshipTest, SetTypeUpdatesCorrectly) {
    Relationship rel("source", "target", "", false, 1.0);
    rel.setType("causes");
    EXPECT_EQ(rel.getType(), "causes");
}

TEST_F(RelationshipTest, SetWeightUpdatesCorrectly) {
    Relationship rel("source", "target", "", false, 1.0);
    rel.setWeight(0.75);
    EXPECT_DOUBLE_EQ(rel.getWeight(), 0.75);
}

TEST_F(RelationshipTest, SetDirectedUpdatesCorrectly) {
    Relationship rel("source", "target", "", false, 1.0);
    rel.setDirected(true);
    EXPECT_TRUE(rel.getIsDirected());
}

// Utility method tests
TEST_F(RelationshipTest, ConnectsReturnsTrueForBothConcepts) {
    Relationship rel("concept1", "concept2", "", false, 1.0);
    EXPECT_TRUE(rel.connects("concept1", "concept2"));
    EXPECT_TRUE(rel.connects("concept2", "concept1")); // Undirected
}

TEST_F(RelationshipTest, ConnectsReturnsFalseForUnrelatedConcepts) {
    Relationship rel("concept1", "concept2", "", false, 1.0);
    EXPECT_FALSE(rel.connects("concept1", "concept3"));
    EXPECT_FALSE(rel.connects("concept3", "concept4"));
}

TEST_F(RelationshipTest, ConnectsToReturnsTrueForSource) {
    Relationship rel("source", "target", "", false, 1.0);
    EXPECT_TRUE(rel.connectsTo("source"));
}

TEST_F(RelationshipTest, ConnectsToReturnsTrueForTarget) {
    Relationship rel("source", "target", "", false, 1.0);
    EXPECT_TRUE(rel.connectsTo("target"));
}

TEST_F(RelationshipTest, ConnectsToReturnsFalseForUnrelatedConcept) {
    Relationship rel("source", "target", "", false, 1.0);
    EXPECT_FALSE(rel.connectsTo("other"));
}

TEST_F(RelationshipTest, GetOtherConceptReturnsTarget) {
    Relationship rel("source", "target", "", false, 1.0);
    EXPECT_EQ(rel.getOtherConcept("source"), "target");
}

TEST_F(RelationshipTest, GetOtherConceptReturnsSource) {
    Relationship rel("source", "target", "", false, 1.0);
    EXPECT_EQ(rel.getOtherConcept("target"), "source");
}

TEST_F(RelationshipTest, GetOtherConceptReturnsEmptyForUnrelatedConcept) {
    Relationship rel("source", "target", "", false, 1.0);
    EXPECT_EQ(rel.getOtherConcept("other"), "");
}

// Equality tests
TEST_F(RelationshipTest, EqualityOperatorReturnsTrueForSameId) {
    Relationship rel1("id1", "source", "target", "", false, 1.0);
    Relationship rel2("id1", "different", "concepts", "", false, 1.0);
    EXPECT_TRUE(rel1 == rel2);
}

TEST_F(RelationshipTest, EqualityOperatorReturnsFalseForDifferentId) {
    Relationship rel1("id1", "source", "target", "", false, 1.0);
    Relationship rel2("id2", "source", "target", "", false, 1.0);
    EXPECT_FALSE(rel1 == rel2);
}

// toString test
TEST_F(RelationshipTest, ToStringContainsRelevantInfo) {
    Relationship rel("source", "target", "causes", true, 0.8);
    std::string str = rel.toString();
    EXPECT_NE(str.find("source"), std::string::npos);
    EXPECT_NE(str.find("target"), std::string::npos);
}

// Weight edge cases
TEST_F(RelationshipTest, ZeroWeightIsAllowed) {
    Relationship rel("source", "target", "", false, 0.0);
    EXPECT_DOUBLE_EQ(rel.getWeight(), 0.0);
}

TEST_F(RelationshipTest, NegativeWeightIsAllowed) {
    Relationship rel("source", "target", "", false, -0.5);
    EXPECT_DOUBLE_EQ(rel.getWeight(), -0.5);
}

TEST_F(RelationshipTest, VeryLargeWeightIsHandled) {
    Relationship rel("source", "target", "", false, 1000000.0);
    EXPECT_DOUBLE_EQ(rel.getWeight(), 1000000.0);
}

// Type edge cases
TEST_F(RelationshipTest, EmptyTypeIsAllowed) {
    Relationship rel("source", "target", "", false, 1.0);
    EXPECT_EQ(rel.getType(), "");
}

TEST_F(RelationshipTest, SpecialCharactersInType) {
    Relationship rel("source", "target", "type!@#$%", false, 1.0);
    EXPECT_EQ(rel.getType(), "type!@#$%");
}

TEST_F(RelationshipTest, LongTypeStringIsHandled) {
    std::string longType(1000, 'a');
    Relationship rel("source", "target", longType, false, 1.0);
    EXPECT_EQ(rel.getType(), longType);
}

// Concept ID edge cases
TEST_F(RelationshipTest, SameSourceAndTargetIsAllowed) {
    Relationship rel("concept1", "concept1");
    EXPECT_EQ(rel.getSourceConceptId(), "concept1");
    EXPECT_EQ(rel.getTargetConceptId(), "concept1");
}

TEST_F(RelationshipTest, EmptyConceptIdsAreAllowed) {
    Relationship rel("", "");
    EXPECT_EQ(rel.getSourceConceptId(), "");
    EXPECT_EQ(rel.getTargetConceptId(), "");
}
