cmake_minimum_required(VERSION 3.5)
project(Assignment2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

# Enable Qt's MOC (Meta-Object Compiler)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/core)
include_directories(${CMAKE_SOURCE_DIR}/ui)

# Core library sources
set(CORE_SOURCES
    core/model/Concept.cpp
    core/model/Relationship.cpp
    core/model/MentalModel.cpp
    core/ai/ILinkPredictor.cpp
    core/ai/CommonNeighborPredictor.cpp
    core/ai/JaccardCoefficientPredictor.cpp
    core/ai/PreferentialAttachmentPredictor.cpp
    core/ai/AIAssistant.cpp
    core/nlp/Commands.cpp
    core/nlp/CommandFactory.cpp
    core/persistence/ModelManager.cpp
)

set(CORE_HEADERS
    core/model/Concept.h
    core/model/Relationship.h
    core/model/MentalModel.h
    core/ai/ILinkPredictor.h
    core/ai/CommonNeighborPredictor.h
    core/ai/JaccardCoefficientPredictor.h
    core/ai/PreferentialAttachmentPredictor.h
    core/ai/AIAssistant.h
    core/nlp/ICommand.h
    core/nlp/Commands.h
    core/nlp/CommandFactory.h
    core/persistence/ModelManager.h
    core/common/QLinkException.h
    core/common/DataStructures.h
)

# UI sources
set(UI_SOURCES
    ui/MainWindow.cpp
    ui/GraphWidget.cpp
    ui/SuggestionPanel.cpp
    main.cpp
)

set(UI_HEADERS
    ui/MainWindow.h
    ui/GraphWidget.h
    ui/SuggestionPanel.h
)

# Create core library
add_library(Assignment2Core STATIC ${CORE_SOURCES} ${CORE_HEADERS})
target_link_libraries(Assignment2Core Qt6::Core Qt6::Network)

# Create main executable
add_executable(assignment2 ${UI_SOURCES} ${UI_HEADERS})
target_link_libraries(assignment2 Assignment2Core Qt6::Core Qt6::Widgets Qt6::Network)

# Enable testing
# enable_testing()

# Find GoogleTest (optional for now)
# find_package(GTest QUIET)
# if(GTest_FOUND)
#     add_subdirectory(tests)
# endif()

# Set output directories
set_target_properties(assignment2 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(Assignment2Core PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)
