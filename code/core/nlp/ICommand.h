#pragma once

#include <string>

namespace qlink {

/**
 * Interface for command pattern implementation
 * Supports undo/redo functionality
 */
class ICommand {
public:
    virtual ~ICommand() = default;
    
    /**
     * Execute the command
     */
    virtual void execute() = 0;
    
    /**
     * Undo the command (reverse the operation)
     */
    virtual void undo() = 0;
    
    /**
     * Get a description of what this command does
     * @return Human readable description of the command
     */
    virtual std::string getDescription() const = 0;
    
    /**
     * Check if this command can be undone
     * @return true if the command supports undo, false otherwise
     */
    virtual bool canUndo() const { return true; }
};

} // namespace qlink