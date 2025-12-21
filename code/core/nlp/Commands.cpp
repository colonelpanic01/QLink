#include "Commands.h"
#include "../model/MentalModel.h"
#include "../model/Concept.h"
#include "../model/Relationship.h"
#include <memory>

namespace qlink {

// AddConceptCommand
AddConceptCommand::AddConceptCommand(MentalModel* model, const std::string& name, const std::string& description)
    : model(model), conceptName(name), conceptDescription(description) {
}

void AddConceptCommand::execute() {
    auto concept = std::make_unique<Concept>(conceptName, conceptDescription);
    addedConceptId = concept->getId();
    model->addConcept(std::move(concept));
}

void AddConceptCommand::undo() {
    if (!addedConceptId.empty()) {
        model->removeConcept(addedConceptId);
        addedConceptId.clear();
    }
}

std::string AddConceptCommand::getDescription() const {
    return "Add concept: " + conceptName;
}

// RemoveConceptCommand
RemoveConceptCommand::RemoveConceptCommand(MentalModel* model, const std::string& conceptId)
    : model(model), conceptId(conceptId) {
}

void RemoveConceptCommand::execute() {
    // Save concept and its relationships for undo
    const Concept* concept = model->getConcept(conceptId);
    if (concept) {
        removedConcept = std::make_unique<Concept>(*concept);
        
        // Save all relationships connected to this concept
        auto relationships = model->getConceptRelationships(conceptId);
        for (const auto& rel : relationships) {
            removedRelationships.push_back(std::make_unique<Relationship>(*rel));
        }
        
        model->removeConcept(conceptId);
    }
}

void RemoveConceptCommand::undo() {
    if (removedConcept) {
        // Restore the concept
        model->addConcept(std::make_unique<Concept>(*removedConcept));
        
        // Restore all relationships
        for (auto& rel : removedRelationships) {
            model->addRelationship(std::make_unique<Relationship>(*rel));
        }
        
        removedConcept.reset();
        removedRelationships.clear();
    }
}

std::string RemoveConceptCommand::getDescription() const {
    return "Remove concept: " + conceptId;
}

// CreateRelationshipCommand
CreateRelationshipCommand::CreateRelationshipCommand(MentalModel* model, const std::string& sourceId, 
                                                   const std::string& targetId, const std::string& type, bool directed)
    : model(model), sourceConceptId(sourceId), targetConceptId(targetId), relationshipType(type), isDirected(directed) {
}

void CreateRelationshipCommand::execute() {
    auto relationship = std::make_unique<Relationship>(sourceConceptId, targetConceptId, relationshipType, isDirected, 1.0);
    addedRelationshipId = relationship->getId();
    model->addRelationship(std::move(relationship));
}

void CreateRelationshipCommand::undo() {
    if (!addedRelationshipId.empty()) {
        model->removeRelationship(addedRelationshipId);
        addedRelationshipId.clear();
    }
}

std::string CreateRelationshipCommand::getDescription() const {
    return "Create relationship: " + sourceConceptId + " -> " + targetConceptId;
}

// DeleteRelationshipCommand
DeleteRelationshipCommand::DeleteRelationshipCommand(MentalModel* model, const std::string& relationshipId)
    : model(model), relationshipId(relationshipId) {
}

void DeleteRelationshipCommand::execute() {
    const Relationship* rel = model->getRelationship(relationshipId);
    if (rel) {
        removedRelationship = std::make_unique<Relationship>(*rel);
        model->removeRelationship(relationshipId);
    }
}

void DeleteRelationshipCommand::undo() {
    if (removedRelationship) {
        model->addRelationship(std::make_unique<Relationship>(*removedRelationship));
        removedRelationship.reset();
    }
}

std::string DeleteRelationshipCommand::getDescription() const {
    return "Delete relationship: " + relationshipId;
}

} // namespace qlink