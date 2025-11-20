#pragma once

#include <string>
#include <memory>
#include <vector>

class QString;

namespace qlink {

// Forward declarations
class Concept;

/**
 * AI assistant for generating explanations and suggestions using Cohere API
 */
class AIAssistant {
public:
    AIAssistant();
    ~AIAssistant();
    
    /**
     * Generate explanation for why two concepts might be connected
     */
    std::string explainConnection(const Concept& concept1, const Concept& concept2);
    
    /**
     * Generate description for a concept based on its name
     */
    std::string generateConceptDescription(const std::string& conceptName);
    
    /**
     * Suggest related concepts for a given concept
     */
    std::vector<std::string> suggestRelatedConcepts(const Concept& concept);
    
    /**
     * Check if AI service is available
     */
    bool isServiceAvailable() const;
    
    /**
     * Configuration methods
     */
    void setApiKey(const QString& apiKey);
    void setApiEndpoint(const QString& endpoint);
    void setTimeout(int milliseconds);
    
    /**
     * Cache management
     */
    void clearCache();
    size_t getCacheSize() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

}