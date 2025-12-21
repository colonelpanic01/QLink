#pragma once

#include "ICommand.h"
#include <string>
#include <memory>

namespace qlink {

// Forward declaration
class MentalModel;

/**
 * Factory class for creating commands from natural language input
 */
class CommandFactory {
public:
    /**
     * Create a command from natural language input
     * @param input The natural language command string
     * @param model The mental model to operate on
     * @return A command object, or nullptr if not implemented yet
     */
    static std::unique_ptr<ICommand> createCommand(const std::string& input, MentalModel* model);
    
    /**
     * Check if the input represents a valid command
     * @param input The input string to check
     * @return true if the input can be parsed as a command
     */
    static bool isValidCommand(const std::string& input);
    
    /**
     * Get help text for available commands
     * @return Help text string
     */
    static std::string getHelpText();
};

} // namespace qlink