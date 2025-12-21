#include <gtest/gtest.h>
#include "../../core/nlp/Commands.h"
#include "../../core/model/MentalModel.h"
#include "../../core/model/Concept.h"
#include "../../core/model/Relationship.h"
#include <memory>

using namespace qlink;

class CommandsTest : public ::testing::Test {
protected:
    void SetUp() override {
        model = std::make_unique<MentalModel>("Test Model");
    }
    
    std::unique_ptr<MentalModel> model;
};

// AddConceptCommand Tests
TEST_F(CommandsTest, AddConceptExecuteAddsConceptToModel) {
    AddConceptCommand cmd(model.get(), "TestConcept", "Test description");
    
    size_t initialSize = model->getConcepts().size();
    cmd.execute();
    
    EXPECT_EQ(model->getConcepts().size(), initialSize + 1);
    
    // Find the added concept
    bool found = false;
    for (const auto& concept : model->getConcepts()) {
        if (concept->getName() == "TestConcept") {
            found = true;
            EXPECT_EQ(concept->getDescription(), "Test description");
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(CommandsTest, AddConceptUndoRemovesConcept) {
    AddConceptCommand cmd(model.get(), "TestConcept", "Test description");
    
    cmd.execute();
    size_t sizeAfterExecute = model->getConcepts().size();
    
    cmd.undo();
    
    EXPECT_LT(model->getConcepts().size(), sizeAfterExecute);
    
    // Verify concept is gone
    for (const auto& concept : model->getConcepts()) {
        EXPECT_NE(concept->getName(), "TestConcept");
    }
}

TEST_F(CommandsTest, AddConceptWithEmptyDescription) {
    AddConceptCommand cmd(model.get(), "MinimalConcept");
    
    cmd.execute();
    
    bool found = false;
    for (const auto& concept : model->getConcepts()) {
        if (concept->getName() == "MinimalConcept") {
            found = true;
            EXPECT_EQ(concept->getDescription(), "");
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(CommandsTest, AddConceptGetDescriptionReturnsCorrectString) {
    AddConceptCommand cmd(model.get(), "MyTest");
    
    std::string desc = cmd.getDescription();
    EXPECT_FALSE(desc.empty());
    EXPECT_NE(desc.find("MyTest"), std::string::npos);
}

TEST_F(CommandsTest, AddConceptMultipleExecuteUndo) {
    AddConceptCommand cmd(model.get(), "Concept1");
    
    cmd.execute();
    size_t afterFirst = model->getConcepts().size();
    
    cmd.undo();
    size_t afterUndo = model->getConcepts().size();
    
    cmd.execute();
    size_t afterSecondExecute = model->getConcepts().size();
    
    EXPECT_EQ(afterFirst, afterSecondExecute);
    EXPECT_LT(afterUndo, afterFirst);
}

// RemoveConceptCommand Tests
TEST_F(CommandsTest, RemoveConceptExecuteRemovesConcept) {
    auto concept = std::make_unique<Concept>("ToRemove");
    std::string conceptId = concept->getId();
    model->addConcept(std::move(concept));
    
    size_t initialSize = model->getConcepts().size();
    
    RemoveConceptCommand cmd(model.get(), conceptId);
    cmd.execute();
    
    EXPECT_EQ(model->getConcepts().size(), initialSize - 1);
    EXPECT_EQ(model->getConcept(conceptId), nullptr);
}

TEST_F(CommandsTest, RemoveConceptUndoRestoresConcept) {
    auto concept = std::make_unique<Concept>("ToRemove", "Important data");
    std::string conceptId = concept->getId();
    std::string conceptName = concept->getName();
    model->addConcept(std::move(concept));
    
    RemoveConceptCommand cmd(model.get(), conceptId);
    cmd.execute();
    cmd.undo();
    
    const Concept* restored = model->getConcept(conceptId);
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->getName(), conceptName);
    EXPECT_EQ(restored->getDescription(), "Important data");
}

TEST_F(CommandsTest, RemoveConceptAlsoRemovesRelationships) {
    auto c1 = std::make_unique<Concept>("C1");
    auto c2 = std::make_unique<Concept>("C2");
    std::string c1Id = c1->getId();
    std::string c2Id = c2->getId();
    
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    
    auto rel = std::make_unique<Relationship>(c1Id, c2Id, "", false, 1.0);
    model->addRelationship(std::move(rel));
    
    size_t initialRelCount = model->getRelationships().size();
    
    RemoveConceptCommand cmd(model.get(), c1Id);
    cmd.execute();
    
    // Relationships connected to removed concept should be gone
    EXPECT_LT(model->getRelationships().size(), initialRelCount);
}

TEST_F(CommandsTest, RemoveConceptUndoRestoresRelationships) {
    auto c1 = std::make_unique<Concept>("C1");
    auto c2 = std::make_unique<Concept>("C2");
    std::string c1Id = c1->getId();
    std::string c2Id = c2->getId();
    
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    
    auto rel = std::make_unique<Relationship>(c1Id, c2Id, "test_type", false, 1.0);
    std::string relId = rel->getId();
    model->addRelationship(std::move(rel));
    
    RemoveConceptCommand cmd(model.get(), c1Id);
    cmd.execute();
    cmd.undo();
    
    // Relationships should be restored
    const Relationship* restored = model->getRelationship(relId);
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->getType(), "test_type");
}

TEST_F(CommandsTest, RemoveConceptOnNonexistentDoesNotCrash) {
    RemoveConceptCommand cmd(model.get(), "nonexistent-id");
    EXPECT_NO_THROW(cmd.execute());
    EXPECT_NO_THROW(cmd.undo());
}

TEST_F(CommandsTest, RemoveConceptGetDescriptionReturnsCorrectString) {
    RemoveConceptCommand cmd(model.get(), "some-id");
    
    std::string desc = cmd.getDescription();
    EXPECT_FALSE(desc.empty());
    EXPECT_NE(desc.find("some-id"), std::string::npos);
}

// CreateRelationshipCommand Tests
TEST_F(CommandsTest, CreateRelationshipExecuteAddsRelationship) {
    auto c1 = std::make_unique<Concept>("C1");
    auto c2 = std::make_unique<Concept>("C2");
    std::string c1Id = c1->getId();
    std::string c2Id = c2->getId();
    
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    
    size_t initialSize = model->getRelationships().size();
    
    CreateRelationshipCommand cmd(model.get(), c1Id, c2Id, "relates_to", false);
    cmd.execute();
    
    EXPECT_EQ(model->getRelationships().size(), initialSize + 1);
    EXPECT_TRUE(model->areConnected(c1Id, c2Id));
}

TEST_F(CommandsTest, CreateRelationshipUndoRemovesRelationship) {
    auto c1 = std::make_unique<Concept>("C1");
    auto c2 = std::make_unique<Concept>("C2");
    std::string c1Id = c1->getId();
    std::string c2Id = c2->getId();
    
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    
    CreateRelationshipCommand cmd(model.get(), c1Id, c2Id, "relates_to", false);
    cmd.execute();
    
    size_t sizeAfterExecute = model->getRelationships().size();
    
    cmd.undo();
    
    EXPECT_LT(model->getRelationships().size(), sizeAfterExecute);
    EXPECT_FALSE(model->areConnected(c1Id, c2Id));
}

TEST_F(CommandsTest, CreateRelationshipWithDirectedFlag) {
    auto c1 = std::make_unique<Concept>("C1");
    auto c2 = std::make_unique<Concept>("C2");
    std::string c1Id = c1->getId();
    std::string c2Id = c2->getId();
    
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    
    CreateRelationshipCommand cmd(model.get(), c1Id, c2Id, "points_to", true);
    cmd.execute();
    
    // Find the relationship and verify it's directed
    bool found = false;
    for (const auto& rel : model->getRelationships()) {
        if (rel->getSourceConceptId() == c1Id && rel->getTargetConceptId() == c2Id) {
            found = true;
            EXPECT_TRUE(rel->getIsDirected());
            EXPECT_EQ(rel->getType(), "points_to");
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(CommandsTest, CreateRelationshipWithEmptyType) {
    auto c1 = std::make_unique<Concept>("C1");
    auto c2 = std::make_unique<Concept>("C2");
    std::string c1Id = c1->getId();
    std::string c2Id = c2->getId();
    
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    
    CreateRelationshipCommand cmd(model.get(), c1Id, c2Id);
    EXPECT_NO_THROW(cmd.execute());
    
    EXPECT_TRUE(model->areConnected(c1Id, c2Id));
}

TEST_F(CommandsTest, CreateRelationshipGetDescriptionReturnsCorrectString) {
    CreateRelationshipCommand cmd(model.get(), "id1", "id2", "type", false);
    
    std::string desc = cmd.getDescription();
    EXPECT_FALSE(desc.empty());
    EXPECT_NE(desc.find("id1"), std::string::npos);
    EXPECT_NE(desc.find("id2"), std::string::npos);
}

TEST_F(CommandsTest, CreateRelationshipMultipleExecuteUndo) {
    auto c1 = std::make_unique<Concept>("C1");
    auto c2 = std::make_unique<Concept>("C2");
    std::string c1Id = c1->getId();
    std::string c2Id = c2->getId();
    
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    
    CreateRelationshipCommand cmd(model.get(), c1Id, c2Id, "test", false);
    
    cmd.execute();
    size_t afterFirst = model->getRelationships().size();
    
    cmd.undo();
    size_t afterUndo = model->getRelationships().size();
    
    cmd.execute();
    size_t afterSecond = model->getRelationships().size();
    
    EXPECT_EQ(afterFirst, afterSecond);
    EXPECT_LT(afterUndo, afterFirst);
}

// DeleteRelationshipCommand Tests
TEST_F(CommandsTest, DeleteRelationshipExecuteRemovesRelationship) {
    auto c1 = std::make_unique<Concept>("C1");
    auto c2 = std::make_unique<Concept>("C2");
    std::string c1Id = c1->getId();
    std::string c2Id = c2->getId();
    
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    
    auto rel = std::make_unique<Relationship>(c1Id, c2Id, "", false, 1.0);
    std::string relId = rel->getId();
    model->addRelationship(std::move(rel));
    
    size_t initialSize = model->getRelationships().size();
    
    DeleteRelationshipCommand cmd(model.get(), relId);
    cmd.execute();
    
    EXPECT_EQ(model->getRelationships().size(), initialSize - 1);
    EXPECT_EQ(model->getRelationship(relId), nullptr);
}

TEST_F(CommandsTest, DeleteRelationshipUndoRestoresRelationship) {
    auto c1 = std::make_unique<Concept>("C1");
    auto c2 = std::make_unique<Concept>("C2");
    std::string c1Id = c1->getId();
    std::string c2Id = c2->getId();
    
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    
    auto rel = std::make_unique<Relationship>(c1Id, c2Id, "special_type", true, 2.5);
    std::string relId = rel->getId();
    model->addRelationship(std::move(rel));
    
    DeleteRelationshipCommand cmd(model.get(), relId);
    cmd.execute();
    cmd.undo();
    
    const Relationship* restored = model->getRelationship(relId);
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->getType(), "special_type");
    EXPECT_TRUE(restored->getIsDirected());
    EXPECT_DOUBLE_EQ(restored->getWeight(), 2.5);
}

TEST_F(CommandsTest, DeleteRelationshipOnNonexistentDoesNotCrash) {
    DeleteRelationshipCommand cmd(model.get(), "nonexistent-id");
    EXPECT_NO_THROW(cmd.execute());
    EXPECT_NO_THROW(cmd.undo());
}

TEST_F(CommandsTest, DeleteRelationshipGetDescriptionReturnsCorrectString) {
    DeleteRelationshipCommand cmd(model.get(), "rel-id-123");
    
    std::string desc = cmd.getDescription();
    EXPECT_FALSE(desc.empty());
    EXPECT_NE(desc.find("rel-id-123"), std::string::npos);
}

// Integration Tests
TEST_F(CommandsTest, ComplexScenarioWithMultipleCommands) {
    // Add concepts
    AddConceptCommand addC1(model.get(), "Concept1", "First concept");
    AddConceptCommand addC2(model.get(), "Concept2", "Second concept");
    
    addC1.execute();
    addC2.execute();
    
    EXPECT_EQ(model->getConcepts().size(), 2);
    
    // Get IDs
    std::string c1Id, c2Id;
    for (const auto& concept : model->getConcepts()) {
        if (concept->getName() == "Concept1") c1Id = concept->getId();
        if (concept->getName() == "Concept2") c2Id = concept->getId();
    }
    
    // Create relationship
    CreateRelationshipCommand createRel(model.get(), c1Id, c2Id, "links", false);
    createRel.execute();
    
    EXPECT_EQ(model->getRelationships().size(), 1);
    EXPECT_TRUE(model->areConnected(c1Id, c2Id));
    
    // Undo relationship
    createRel.undo();
    EXPECT_EQ(model->getRelationships().size(), 0);
    
    // Undo concepts
    addC2.undo();
    EXPECT_EQ(model->getConcepts().size(), 1);
    
    addC1.undo();
    EXPECT_EQ(model->getConcepts().size(), 0);
}

TEST_F(CommandsTest, UndoWithoutExecuteDoesNothing) {
    AddConceptCommand cmd(model.get(), "Test");
    
    size_t initialSize = model->getConcepts().size();
    cmd.undo();
    
    EXPECT_EQ(model->getConcepts().size(), initialSize);
}

TEST_F(CommandsTest, MultipleUndoCallsAreSafe) {
    AddConceptCommand cmd(model.get(), "Test");
    
    cmd.execute();
    cmd.undo();
    
    size_t sizeAfterFirstUndo = model->getConcepts().size();
    
    EXPECT_NO_THROW(cmd.undo());
    EXPECT_EQ(model->getConcepts().size(), sizeAfterFirstUndo);
}
