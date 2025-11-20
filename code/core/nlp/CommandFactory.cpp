#include "CommandFactory.h"

namespace qlink {

std::unique_ptr<ICommand> CommandFactory::createCommand(const std::string& input, MentalModel* model) {
    // TODO: Implement natural language command parsing in next deliverable
    // This is a placeholder for future NLP functionality
    return nullptr;
}

bool CommandFactory::isValidCommand(const std::string& input) {
    // TODO: Implement command validation in next deliverable
    return false;
}

std::string CommandFactory::getHelpText() {
    return "Natural language command processing will be implemented in the next deliverable.\n"
           "For now, please use the GUI interface to interact with the mental model.";
}

} // namespace qlink