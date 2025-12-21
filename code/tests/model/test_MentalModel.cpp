#include <gtest/gtest.h>
#include "../../core/model/MentalModel.h"
#include "../../core/model/Concept.h"
#include "../../core/model/Relationship.h"

using namespace qlink;

class MentalModelTest : public ::testing::Test {
protected:
    void SetUp() override {
        model = std::make_unique<MentalModel>("Test Model");
    }
    
    std::unique_ptr<MentalModel> model;
};

// Initial state tests
TEST_F(MentalModelTest, InitialStateIsEmpty) {
    EXPECT_TRUE(model->isEmpty());
    EXPECT_EQ(model->getConceptCount(), 0);
    EXPECT_EQ(model->getRelationshipCount(), 0);
}

TEST_F(MentalModelTest, ModelNameIsSetCorrectly) {
    EXPECT_EQ(model->getModelName(), "Test Model");
}

// Concept management tests
TEST_F(MentalModelTest, CanAddConcept) {
    auto concept = std::make_unique<Concept>("Test Concept");
    std::string conceptId = concept->getId();
    
    model->addConcept(std::move(concept));
    
    EXPECT_EQ(model->getConceptCount(), 1);
    EXPECT_FALSE(model->isEmpty());
    EXPECT_NE(model->getConcept(conceptId), nullptr);
}

TEST_F(MentalModelTest, CanAddMultipleConcepts) {
    auto concept1 = std::make_unique<Concept>("Concept1");
    auto concept2 = std::make_unique<Concept>("Concept2");
    auto concept3 = std::make_unique<Concept>("Concept3");
    
    model->addConcept(std::move(concept1));
    model->addConcept(std::move(concept2));
    model->addConcept(std::move(concept3));
    
    EXPECT_EQ(model->getConceptCount(), 3);
}

TEST_F(MentalModelTest, GetConceptReturnsNullForNonexistentId) {
    EXPECT_EQ(model->getConcept("nonexistent"), nullptr);
}

TEST_F(MentalModelTest, CanRemoveConcept) {
    auto concept = std::make_unique<Concept>("Test");
    std::string conceptId = concept->getId();
    
    model->addConcept(std::move(concept));
    EXPECT_EQ(model->getConceptCount(), 1);
    
    model->removeConcept(conceptId);
    EXPECT_EQ(model->getConceptCount(), 0);
    EXPECT_EQ(model->getConcept(conceptId), nullptr);
}

TEST_F(MentalModelTest, RemoveNonexistentConceptDoesNotCrash) {
    EXPECT_NO_THROW(model->removeConcept("nonexistent"));
}

TEST_F(MentalModelTest, GetConceptsReturnsAllConcepts) {
    model->addConcept(std::make_unique<Concept>("C1"));
    model->addConcept(std::make_unique<Concept>("C2"));
    model->addConcept(std::make_unique<Concept>("C3"));
    
    const auto& concepts = model->getConcepts();
    EXPECT_EQ(concepts.size(), 3);
}

// Relationship management tests
TEST_F(MentalModelTest, CanAddRelationship) {
    auto c1 = std::make_unique<Concept>("Concept1");
    auto c2 = std::make_unique<Concept>("Concept2");
    std::string c1Id = c1->getId();
    std::string c2Id = c2->getId();
    
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    
    auto rel = std::unique_ptr<Relationship>(new Relationship(c1Id, c2Id, "causes", false, 1.0));
    std::string relId = rel->getId();
    
    model->addRelationship(std::move(rel));
    
    EXPECT_EQ(model->getRelationshipCount(), 1);
    EXPECT_NE(model->getRelationship(relId), nullptr);
}

TEST_F(MentalModelTest, CanAddMultipleRelationships) {
    auto c1 = std::make_unique<Concept>("C1");
    auto c2 = std::make_unique<Concept>("C2");
    auto c3 = std::make_unique<Concept>("C3");
    std::string c1Id = c1->getId();
    std::string c2Id = c2->getId();
    std::string c3Id = c3->getId();
    
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    model->addConcept(std::move(c3));
    
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(c1Id, c2Id)));
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(c2Id, c3Id)));
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(c1Id, c3Id)));
    
    EXPECT_EQ(model->getRelationshipCount(), 3);
}

TEST_F(MentalModelTest, GetRelationshipReturnsNullForNonexistentId) {
    EXPECT_EQ(model->getRelationship("nonexistent"), nullptr);
}

TEST_F(MentalModelTest, CanRemoveRelationship) {
    auto c1 = std::make_unique<Concept>("C1");
    auto c2 = std::make_unique<Concept>("C2");
    std::string c1Id = c1->getId();
    std::string c2Id = c2->getId();
    
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    
    auto rel = std::unique_ptr<Relationship>(new Relationship(c1Id, c2Id));
    std::string relId = rel->getId();
    
    model->addRelationship(std::move(rel));
    EXPECT_EQ(model->getRelationshipCount(), 1);
    
    model->removeRelationship(relId);
    EXPECT_EQ(model->getRelationshipCount(), 0);
}

TEST_F(MentalModelTest, RemoveNonexistentRelationshipDoesNotCrash) {
    EXPECT_NO_THROW(model->removeRelationship("nonexistent"));
}

// Graph operation tests
TEST_F(MentalModelTest, AreConnectedReturnsFalseForUnconnected) {
    auto c1 = std::make_unique<Concept>("C1");
    auto c2 = std::make_unique<Concept>("C2");
    std::string c1Id = c1->getId();
    std::string c2Id = c2->getId();
    
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    
    EXPECT_FALSE(model->areConnected(c1Id, c2Id));
}

TEST_F(MentalModelTest, AreConnectedReturnsTrueForConnected) {
    auto c1 = std::make_unique<Concept>("C1");
    auto c2 = std::make_unique<Concept>("C2");
    std::string c1Id = c1->getId();
    std::string c2Id = c2->getId();
    
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(c1Id, c2Id)));
    
    EXPECT_TRUE(model->areConnected(c1Id, c2Id));
}

TEST_F(MentalModelTest, GetConnectedConceptsReturnsEmptyForIsolatedConcept) {
    auto c1 = std::make_unique<Concept>("C1");
    std::string c1Id = c1->getId();
    model->addConcept(std::move(c1));
    
    auto connected = model->getConnectedConcepts(c1Id);
    EXPECT_EQ(connected.size(), 0);
}

TEST_F(MentalModelTest, GetConnectedConceptsReturnsAllConnections) {
    auto c1 = std::make_unique<Concept>("C1");
    auto c2 = std::make_unique<Concept>("C2");
    auto c3 = std::make_unique<Concept>("C3");
    std::string c1Id = c1->getId();
    std::string c2Id = c2->getId();
    std::string c3Id = c3->getId();
    
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    model->addConcept(std::move(c3));
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(c1Id, c2Id)));
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(c1Id, c3Id)));
    
    auto connected = model->getConnectedConcepts(c1Id);
    EXPECT_EQ(connected.size(), 2);
}

TEST_F(MentalModelTest, GetConceptRelationshipsReturnsAllRelationships) {
    auto c1 = std::make_unique<Concept>("C1");
    auto c2 = std::make_unique<Concept>("C2");
    auto c3 = std::make_unique<Concept>("C3");
    std::string c1Id = c1->getId();
    std::string c2Id = c2->getId();
    std::string c3Id = c3->getId();
    
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    model->addConcept(std::move(c3));
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(c1Id, c2Id)));
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(c1Id, c3Id)));
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(c2Id, c3Id)));
    
    auto rels = model->getConceptRelationships(c1Id);
    EXPECT_EQ(rels.size(), 2);
}

TEST_F(MentalModelTest, GetOrphanedConceptsReturnsIsolatedConcepts) {
    auto c1 = std::make_unique<Concept>("C1");
    auto c2 = std::make_unique<Concept>("C2");
    auto c3 = std::make_unique<Concept>("C3");
    std::string c1Id = c1->getId();
    std::string c2Id = c2->getId();
    
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    model->addConcept(std::move(c3));
    model->addRelationship(std::unique_ptr<Relationship>(new Relationship(c1Id, c2Id)));
    
    auto orphans = model->getOrphanedConcepts();
    EXPECT_EQ(orphans.size(), 1);
}

// Model properties tests
TEST_F(MentalModelTest, SetModelNameUpdatesCorrectly) {
    model->setModelName("New Name");
    EXPECT_EQ(model->getModelName(), "New Name");
}

TEST_F(MentalModelTest, ClearRemovesAllContent) {
    model->addConcept(std::make_unique<Concept>("C1"));
    model->addConcept(std::make_unique<Concept>("C2"));
    
    model->clear();
    
    EXPECT_TRUE(model->isEmpty());
    EXPECT_EQ(model->getConceptCount(), 0);
    EXPECT_EQ(model->getRelationshipCount(), 0);
}

// Statistics tests
TEST_F(MentalModelTest, GetStatisticsReturnsCorrectCounts) {
    model->addConcept(std::make_unique<Concept>("C1"));
    model->addConcept(std::make_unique<Concept>("C2"));
    
    auto stats = model->getStatistics();
    EXPECT_EQ(stats.conceptCount, 2);
    EXPECT_EQ(stats.relationshipCount, 0);
}

// Validation tests
TEST_F(MentalModelTest, EmptyModelIsValid) {
    EXPECT_TRUE(model->isValid());
}

TEST_F(MentalModelTest, ModelWithConceptsIsValid) {
    model->addConcept(std::make_unique<Concept>("C1"));
    EXPECT_TRUE(model->isValid());
}

// Edge cases
TEST_F(MentalModelTest, CanHandleLargeNumberOfConcepts) {
    for (int i = 0; i < 1000; ++i) {
        model->addConcept(std::make_unique<Concept>("Concept" + std::to_string(i)));
    }
    EXPECT_EQ(model->getConceptCount(), 1000);
}

TEST_F(MentalModelTest, EmptyModelNameIsAllowed) {
    MentalModel emptyNameModel("");
    EXPECT_EQ(emptyNameModel.getModelName(), "");
}

TEST_F(MentalModelTest, GetConceptImportanceReturnsValueForExistingConcept) {
    auto c1 = std::make_unique<Concept>("C1");
    std::string c1Id = c1->getId();
    model->addConcept(std::move(c1));
    
    double importance = model->getConceptImportance(c1Id);
    EXPECT_GE(importance, 0.0);
}
