#pragma once

#include <exception>
#include <string>

namespace qlink {

/**
 * Base exception class for all the Qlink specific exceptions
 */
class QLinkException : public std::exception {
protected:
    std::string message;

public:
    explicit QLinkException(const std::string& msg) : message(msg) {}
    
    const char* what() const noexcept override {
        return message.c_str();
    }
    
    const std::string& getMessage() const {
        return message;
    }
};

/**
 * Exception for any of the model related errors
 */
class ModelException : public QLinkException {
public:
    explicit ModelException(const std::string& msg) 
        : QLinkException("Model Error: " + msg) {}
};

/**
 * Exception thrown for ai service related errors
 */
class AIServiceException : public QLinkException {
public:
    explicit AIServiceException(const std::string& msg) 
        : QLinkException("AI Service Error: " + msg) {}
};

/**
 * Exception thrown for file I/O related errors
 */
class FileIOException : public QLinkException {
public:
    explicit FileIOException(const std::string& msg) 
        : QLinkException("File I/O Error: " + msg) {}
};

/**
 * Exception thrown for NLP processing errors
 */
class NLPException : public QLinkException {
public:
    explicit NLPException(const std::string& msg) 
        : QLinkException("NLP Error: " + msg) {}
};

} // namespace qlink