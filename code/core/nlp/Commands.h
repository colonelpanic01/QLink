#pragma once

#include "ICommand.h"
#include <string>

namespace qlink {

// Forward declarations
class MentalModel;

/**
 * Command to add a concept to the mental model
 * TODO: Full implementation in next deliverable
 */
class AddConceptCommand : public ICommand {
public:
    AddConceptCommand(MentalModel* model, const std::string& name, const std::string& description = "");
    
    void execute() override;
    void undo() override;
    std::string getDescription() const override;

private:
    MentalModel* model;
    std::string conceptName;
    std::string conceptDescription;
    std::string addedConceptId; // For undo
};

/**
 * Command to remove a concept from the mental model
 * TODO: Full implementation in next deliverable
 */
class RemoveConceptCommand : public ICommand {
public:
    RemoveConceptCommand(MentalModel* model, const std::string& conceptId);
    
    void execute() override;
    void undo() override;
    std::string getDescription() const override;

private:
    MentalModel* model;
    std::string conceptId;
};

/**
 * Command to create a relationship between concepts
 * TODO: Full implementation in next deliverable
 */
class CreateRelationshipCommand : public ICommand {
public:
    CreateRelationshipCommand(MentalModel* model, const std::string& sourceId, 
                            const std::string& targetId, const std::string& type = "", bool directed = false);
    
    void execute() override;
    void undo() override;
    std::string getDescription() const override;

private:
    MentalModel* model;
    std::string sourceConceptId;
    std::string targetConceptId;
    std::string relationshipType;
    bool isDirected;
    std::string addedRelationshipId; // For undo
};

/**
 * Command to delete a relationship
 * TODO: Full implementation in next deliverable
 */
class DeleteRelationshipCommand : public ICommand {
public:
    DeleteRelationshipCommand(MentalModel* model, const std::string& relationshipId);
    
    void execute() override;
    void undo() override;
    std::string getDescription() const override;

private:
    MentalModel* model;
    std::string relationshipId;
};

} // namespace qlink