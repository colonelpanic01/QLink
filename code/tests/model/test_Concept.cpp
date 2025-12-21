#include <gtest/gtest.h>
#include "../../core/model/Concept.h"

using namespace qlink;

class ConceptTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code for each test
    }
    
    void TearDown() override {
        // Cleanup code for each test
    }
};

// Constructor tests
TEST_F(ConceptTest, ConstructorSetsNameCorrectly) {
    Concept concept("Energy", "The capacity to do work");
    EXPECT_EQ(concept.getName(), "Energy");
    EXPECT_EQ(concept.getDescription(), "The capacity to do work");
}

TEST_F(ConceptTest, ConstructorWithEmptyDescription) {
    Concept concept("Energy");
    EXPECT_EQ(concept.getName(), "Energy");
    EXPECT_EQ(concept.getDescription(), "");
}

TEST_F(ConceptTest, ConstructorWithCustomId) {
    Concept concept("custom-id", "Energy", "The capacity to do work");
    EXPECT_EQ(concept.getId(), "custom-id");
    EXPECT_EQ(concept.getName(), "Energy");
    EXPECT_EQ(concept.getDescription(), "The capacity to do work");
}

TEST_F(ConceptTest, IdIsUniqueForEachConcept) {
    Concept concept1("Concept1", "");
    Concept concept2("Concept2", "");
    EXPECT_NE(concept1.getId(), concept2.getId());
}

TEST_F(ConceptTest, IdIsNotEmpty) {
    Concept concept("Test");
    EXPECT_FALSE(concept.getId().empty());
}

// Setter tests
TEST_F(ConceptTest, SetNameUpdatesCorrectly) {
    Concept concept("Old Name");
    concept.setName("New Name");
    EXPECT_EQ(concept.getName(), "New Name");
}

TEST_F(ConceptTest, SetDescriptionUpdatesCorrectly) {
    Concept concept("Test");
    concept.setDescription("New description");
    EXPECT_EQ(concept.getDescription(), "New description");
}

TEST_F(ConceptTest, SetPositionUpdatesCorrectly) {
    Concept concept("Test");
    Position pos(100.0, 200.0);
    concept.setPosition(pos);
    EXPECT_EQ(concept.getPosition().x, 100.0);
    EXPECT_EQ(concept.getPosition().y, 200.0);
}

// Tag management tests
TEST_F(ConceptTest, AddTagWorks) {
    Concept concept("Test");
    concept.addTag("important");
    EXPECT_TRUE(concept.hasTag("important"));
    EXPECT_EQ(concept.getTags().size(), 1);
}

TEST_F(ConceptTest, AddMultipleTagsWorks) {
    Concept concept("Test");
    concept.addTag("tag1");
    concept.addTag("tag2");
    concept.addTag("tag3");
    EXPECT_TRUE(concept.hasTag("tag1"));
    EXPECT_TRUE(concept.hasTag("tag2"));
    EXPECT_TRUE(concept.hasTag("tag3"));
    EXPECT_EQ(concept.getTags().size(), 3);
}

TEST_F(ConceptTest, RemoveTagWorks) {
    Concept concept("Test");
    concept.addTag("tag1");
    concept.addTag("tag2");
    concept.removeTag("tag1");
    EXPECT_FALSE(concept.hasTag("tag1"));
    EXPECT_TRUE(concept.hasTag("tag2"));
    EXPECT_EQ(concept.getTags().size(), 1);
}

TEST_F(ConceptTest, RemoveNonexistentTagDoesNothing) {
    Concept concept("Test");
    concept.addTag("tag1");
    concept.removeTag("nonexistent");
    EXPECT_EQ(concept.getTags().size(), 1);
}

TEST_F(ConceptTest, HasTagReturnsFalseForNonexistentTag) {
    Concept concept("Test");
    EXPECT_FALSE(concept.hasTag("nonexistent"));
}

// Equality tests
TEST_F(ConceptTest, EqualityOperatorReturnsTrueForSameId) {
    Concept concept1("id1", "Name", "Desc");
    Concept concept2("id1", "Different Name", "Different Desc");
    EXPECT_TRUE(concept1 == concept2);
}

TEST_F(ConceptTest, EqualityOperatorReturnsFalseForDifferentId) {
    Concept concept1("id1", "Name", "Desc");
    Concept concept2("id2", "Name", "Desc");
    EXPECT_FALSE(concept1 == concept2);
}

// toString test
TEST_F(ConceptTest, ToStringContainsNameAndDescription) {
    Concept concept("Energy", "The capacity to do work");
    std::string str = concept.toString();
    EXPECT_NE(str.find("Energy"), std::string::npos);
    EXPECT_NE(str.find("The capacity to do work"), std::string::npos);
}

// Edge cases
TEST_F(ConceptTest, EmptyNameIsAllowed) {
    Concept concept("");
    EXPECT_EQ(concept.getName(), "");
}

TEST_F(ConceptTest, VeryLongNameIsHandled) {
    std::string longName(10000, 'a');
    Concept concept(longName);
    EXPECT_EQ(concept.getName(), longName);
}

TEST_F(ConceptTest, SpecialCharactersInName) {
    Concept concept("Test!@#$%^&*()_+-=[]{}|;':\",./<>?");
    EXPECT_EQ(concept.getName(), "Test!@#$%^&*()_+-=[]{}|;':\",./<>?");
}

TEST_F(ConceptTest, UnicodeCharactersInName) {
    Concept concept("能量 エネルギー");
    EXPECT_EQ(concept.getName(), "能量 エネルギー");
}

TEST_F(ConceptTest, PositionDefaultsToOrigin) {
    Concept concept("Test");
    const Position& pos = concept.getPosition();
    EXPECT_EQ(pos.x, 0.0);
    EXPECT_EQ(pos.y, 0.0);
}

TEST_F(ConceptTest, NegativePositionValuesWork) {
    Concept concept("Test");
    Position pos(-100.5, -200.7);
    concept.setPosition(pos);
    EXPECT_EQ(concept.getPosition().x, -100.5);
    EXPECT_EQ(concept.getPosition().y, -200.7);
}
