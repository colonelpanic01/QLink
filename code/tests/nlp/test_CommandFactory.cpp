#include <gtest/gtest.h>
#include "../../core/nlp/CommandFactory.h"
#include "../../core/nlp/Commands.h"
#include "../../core/model/MentalModel.h"
#include "../../core/model/Concept.h"
#include "../../core/model/Relationship.h"
#include <memory>

using namespace qlink;

class CommandFactoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        model = std::make_unique<MentalModel>("Test Model");
    }
    
    std::unique_ptr<MentalModel> model;
};

// isValidCommand tests
TEST_F(CommandFactoryTest, IsValidCommandRecognizesAddKeyword) {
    EXPECT_TRUE(CommandFactory::isValidCommand("add concept Test"));
    EXPECT_TRUE(CommandFactory::isValidCommand("Add Concept Test"));
    EXPECT_TRUE(CommandFactory::isValidCommand("add Test"));
}

TEST_F(CommandFactoryTest, IsValidCommandRecognizesCreateKeyword) {
    EXPECT_TRUE(CommandFactory::isValidCommand("create concept Test"));
    EXPECT_TRUE(CommandFactory::isValidCommand("CREATE CONCEPT Test"));
}

TEST_F(CommandFactoryTest, IsValidCommandRecognizesRemoveKeyword) {
    EXPECT_TRUE(CommandFactory::isValidCommand("remove concept Test"));
    EXPECT_TRUE(CommandFactory::isValidCommand("delete concept Test"));
}

TEST_F(CommandFactoryTest, IsValidCommandRecognizesConnectKeyword) {
    EXPECT_TRUE(CommandFactory::isValidCommand("connect A to B"));
    EXPECT_TRUE(CommandFactory::isValidCommand("link A and B"));
    EXPECT_TRUE(CommandFactory::isValidCommand("relate A with B"));
}

TEST_F(CommandFactoryTest, IsValidCommandRejectsInvalidInput) {
    EXPECT_FALSE(CommandFactory::isValidCommand(""));
    EXPECT_FALSE(CommandFactory::isValidCommand("   "));
    EXPECT_FALSE(CommandFactory::isValidCommand("hello world"));
    EXPECT_FALSE(CommandFactory::isValidCommand("this is not a command"));
}

// getHelpText tests
TEST_F(CommandFactoryTest, GetHelpTextIsNotEmpty) {
    std::string help = CommandFactory::getHelpText();
    EXPECT_FALSE(help.empty());
}

TEST_F(CommandFactoryTest, GetHelpTextContainsKeywords) {
    std::string help = CommandFactory::getHelpText();
    EXPECT_NE(help.find("add"), std::string::npos);
    EXPECT_NE(help.find("concept"), std::string::npos);
    EXPECT_NE(help.find("connect"), std::string::npos);
}

// createCommand - Add Concept tests
TEST_F(CommandFactoryTest, CreateCommandParsesAddConcept) {
    auto cmd = CommandFactory::createCommand("add concept TestConcept", model.get());
    ASSERT_NE(cmd, nullptr);
    
    size_t initialSize = model->getConcepts().size();
    cmd->execute();
    EXPECT_EQ(model->getConcepts().size(), initialSize + 1);
}

TEST_F(CommandFactoryTest, CreateCommandParsesAddConceptWithDescription) {
    auto cmd = CommandFactory::createCommand("create concept AI with description Artificial Intelligence", model.get());
    ASSERT_NE(cmd, nullptr);
    
    cmd->execute();
    
    bool found = false;
    for (const auto& concept : model->getConcepts()) {
        if (concept->getName() == "AI") {
            found = true;
            EXPECT_EQ(concept->getDescription(), "Artificial Intelligence");
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(CommandFactoryTest, CreateCommandParsesSimpleAdd) {
    auto cmd = CommandFactory::createCommand("add MachineLearning", model.get());
    ASSERT_NE(cmd, nullptr);
    
    cmd->execute();
    
    bool found = false;
    for (const auto& concept : model->getConcepts()) {
        if (concept->getName() == "MachineLearning") {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(CommandFactoryTest, CreateCommandHandlesQuotedNames) {
    auto cmd = CommandFactory::createCommand("add concept \"Machine Learning\"", model.get());
    ASSERT_NE(cmd, nullptr);
    
    cmd->execute();
    
    bool found = false;
    for (const auto& concept : model->getConcepts()) {
        if (concept->getName() == "Machine Learning") {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

// createCommand - Remove Concept tests
TEST_F(CommandFactoryTest, CreateCommandParsesRemoveConcept) {
    model->addConcept(std::make_unique<Concept>("TestConcept"));
    
    auto cmd = CommandFactory::createCommand("remove concept TestConcept", model.get());
    ASSERT_NE(cmd, nullptr);
    
    size_t initialSize = model->getConcepts().size();
    cmd->execute();
    EXPECT_EQ(model->getConcepts().size(), initialSize - 1);
}

TEST_F(CommandFactoryTest, CreateCommandParsesDeleteConcept) {
    model->addConcept(std::make_unique<Concept>("ToDelete"));
    
    auto cmd = CommandFactory::createCommand("delete concept ToDelete", model.get());
    ASSERT_NE(cmd, nullptr);
    
    cmd->execute();
    
    bool found = false;
    for (const auto& concept : model->getConcepts()) {
        if (concept->getName() == "ToDelete") {
            found = true;
            break;
        }
    }
    EXPECT_FALSE(found);
}

TEST_F(CommandFactoryTest, CreateCommandRemoveNonexistentReturnsNull) {
    auto cmd = CommandFactory::createCommand("remove concept NonExistent", model.get());
    EXPECT_EQ(cmd, nullptr);
}

// createCommand - Create Relationship tests
TEST_F(CommandFactoryTest, CreateCommandParsesConnectConcepts) {
    model->addConcept(std::make_unique<Concept>("AI"));
    model->addConcept(std::make_unique<Concept>("ML"));
    
    auto cmd = CommandFactory::createCommand("connect AI to ML", model.get());
    ASSERT_NE(cmd, nullptr);
    
    size_t initialSize = model->getRelationships().size();
    cmd->execute();
    EXPECT_EQ(model->getRelationships().size(), initialSize + 1);
}

TEST_F(CommandFactoryTest, CreateCommandParsesLinkConcepts) {
    model->addConcept(std::make_unique<Concept>("Python"));
    model->addConcept(std::make_unique<Concept>("Programming"));
    
    auto cmd = CommandFactory::createCommand("link Python and Programming", model.get());
    ASSERT_NE(cmd, nullptr);
    
    cmd->execute();
    EXPECT_GT(model->getRelationships().size(), 0);
}

TEST_F(CommandFactoryTest, CreateCommandParsesRelateWithType) {
    model->addConcept(std::make_unique<Concept>("Dog"));
    model->addConcept(std::make_unique<Concept>("Animal"));
    
    auto cmd = CommandFactory::createCommand("relate Dog with Animal as is_a", model.get());
    ASSERT_NE(cmd, nullptr);
    
    cmd->execute();
    
    bool found = false;
    for (const auto& rel : model->getRelationships()) {
        if (rel->getType() == "is_a") {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(CommandFactoryTest, CreateCommandParsesDirectedRelationship) {
    model->addConcept(std::make_unique<Concept>("Cause"));
    model->addConcept(std::make_unique<Concept>("Effect"));
    
    auto cmd = CommandFactory::createCommand("connect Cause to Effect directed", model.get());
    ASSERT_NE(cmd, nullptr);
    
    cmd->execute();
    
    bool foundDirected = false;
    for (const auto& rel : model->getRelationships()) {
        if (rel->getIsDirected()) {
            foundDirected = true;
            break;
        }
    }
    EXPECT_TRUE(foundDirected);
}

TEST_F(CommandFactoryTest, CreateCommandConnectNonexistentReturnsNull) {
    model->addConcept(std::make_unique<Concept>("Exists"));
    
    auto cmd = CommandFactory::createCommand("connect Exists to NonExistent", model.get());
    EXPECT_EQ(cmd, nullptr);
}

// createCommand - Delete Relationship tests
TEST_F(CommandFactoryTest, CreateCommandParsesDisconnect) {
    auto c1 = std::make_unique<Concept>("A");
    auto c2 = std::make_unique<Concept>("B");
    std::string id1 = c1->getId();
    std::string id2 = c2->getId();
    
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    model->addRelationship(std::make_unique<Relationship>(id1, id2, "", false, 1.0));
    
    auto cmd = CommandFactory::createCommand("disconnect A from B", model.get());
    ASSERT_NE(cmd, nullptr);
    
    size_t initialSize = model->getRelationships().size();
    cmd->execute();
    EXPECT_EQ(model->getRelationships().size(), initialSize - 1);
}

TEST_F(CommandFactoryTest, CreateCommandParsesRemoveLink) {
    auto c1 = std::make_unique<Concept>("X");
    auto c2 = std::make_unique<Concept>("Y");
    std::string id1 = c1->getId();
    std::string id2 = c2->getId();
    
    model->addConcept(std::move(c1));
    model->addConcept(std::move(c2));
    model->addRelationship(std::make_unique<Relationship>(id1, id2, "", false, 1.0));
    
    auto cmd = CommandFactory::createCommand("remove link between X and Y", model.get());
    ASSERT_NE(cmd, nullptr);
    
    cmd->execute();
    EXPECT_EQ(model->getRelationships().size(), 0);
}

// Case insensitivity tests
TEST_F(CommandFactoryTest, CreateCommandIsCaseInsensitive) {
    auto cmd1 = CommandFactory::createCommand("ADD CONCEPT UPPER", model.get());
    ASSERT_NE(cmd1, nullptr);
    cmd1->execute();
    
    auto cmd2 = CommandFactory::createCommand("add concept lower", model.get());
    ASSERT_NE(cmd2, nullptr);
    cmd2->execute();
    
    auto cmd3 = CommandFactory::createCommand("AdD CoNcEpT MiXeD", model.get());
    ASSERT_NE(cmd3, nullptr);
    cmd3->execute();
    
    EXPECT_EQ(model->getConcepts().size(), 3);
}

TEST_F(CommandFactoryTest, CreateCommandFindsConceptsCaseInsensitively) {
    model->addConcept(std::make_unique<Concept>("MyTest"));
    
    auto cmd = CommandFactory::createCommand("remove concept mytest", model.get());
    ASSERT_NE(cmd, nullptr);
    
    cmd->execute();
    EXPECT_EQ(model->getConcepts().size(), 0);
}

// Edge cases
TEST_F(CommandFactoryTest, CreateCommandReturnsNullForEmptyInput) {
    auto cmd = CommandFactory::createCommand("", model.get());
    EXPECT_EQ(cmd, nullptr);
}

TEST_F(CommandFactoryTest, CreateCommandReturnsNullForWhitespace) {
    auto cmd = CommandFactory::createCommand("   ", model.get());
    EXPECT_EQ(cmd, nullptr);
}

TEST_F(CommandFactoryTest, CreateCommandReturnsNullForInvalidCommand) {
    auto cmd = CommandFactory::createCommand("this is not a valid command", model.get());
    EXPECT_EQ(cmd, nullptr);
}

TEST_F(CommandFactoryTest, CreateCommandReturnsNullForNullModel) {
    auto cmd = CommandFactory::createCommand("add concept Test", nullptr);
    EXPECT_EQ(cmd, nullptr);
}

// Integration test
TEST_F(CommandFactoryTest, CompleteWorkflowWithNaturalLanguage) {
    // Add concepts
    auto cmd1 = CommandFactory::createCommand("add concept AI", model.get());
    ASSERT_NE(cmd1, nullptr);
    cmd1->execute();
    
    auto cmd2 = CommandFactory::createCommand("create concept ML with description Machine Learning", model.get());
    ASSERT_NE(cmd2, nullptr);
    cmd2->execute();
    
    EXPECT_EQ(model->getConcepts().size(), 2);
    
    // Connect them
    auto cmd3 = CommandFactory::createCommand("connect AI to ML as related", model.get());
    ASSERT_NE(cmd3, nullptr);
    cmd3->execute();
    
    EXPECT_EQ(model->getRelationships().size(), 1);
    
    // Disconnect
    auto cmd4 = CommandFactory::createCommand("disconnect AI from ML", model.get());
    ASSERT_NE(cmd4, nullptr);
    cmd4->execute();
    
    EXPECT_EQ(model->getRelationships().size(), 0);
    
    // Remove concepts
    auto cmd5 = CommandFactory::createCommand("remove concept AI", model.get());
    ASSERT_NE(cmd5, nullptr);
    cmd5->execute();
    
    EXPECT_EQ(model->getConcepts().size(), 1);
}
