cmake_minimum_required(VERSION 3.5)
project(Assignment2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

# get igraph package since installation paths vary by system (I used homebrew so the latter only works and pkgconfig cannot find it for the life of me )
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(IGRAPH igraph)
    if(IGRAPH_FOUND)
        set(IGRAPH_INCLUDE_DIRS ${IGRAPH_INCLUDE_DIRS})
        set(IGRAPH_LIBRARIES ${IGRAPH_LIBRARIES})
        set(IGRAPH_LIBRARY_DIRS ${IGRAPH_LIBRARY_DIRS})
    endif()
endif()

if(NOT IGRAPH_FOUND)
    find_path(IGRAPH_INCLUDE_DIR igraph/igraph.h PATHS /usr/include /usr/local/include /opt/homebrew/include)
    find_library(IGRAPH_LIBRARY igraph PATHS /usr/lib /usr/local/lib /opt/homebrew/lib)
    
    if(IGRAPH_INCLUDE_DIR AND IGRAPH_LIBRARY)
        set(IGRAPH_INCLUDE_DIRS ${IGRAPH_INCLUDE_DIR})
        set(IGRAPH_LIBRARIES ${IGRAPH_LIBRARY})
    else()
        message(FATAL_ERROR "igraph not found. Please install igraph library.")
    endif()
endif()

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
target_link_libraries(Assignment2Core Qt6::Core Qt6::Network ${IGRAPH_LIBRARIES})
target_include_directories(Assignment2Core PRIVATE ${IGRAPH_INCLUDE_DIRS})
if(IGRAPH_LIBRARY_DIRS)
    target_link_directories(Assignment2Core PRIVATE ${IGRAPH_LIBRARY_DIRS})
endif()

# Create main executable
add_executable(assignment2 ${UI_SOURCES} ${UI_HEADERS})
target_link_libraries(assignment2 Assignment2Core Qt6::Core Qt6::Widgets Qt6::Network)
if(IGRAPH_LIBRARY_DIRS)
    target_link_directories(assignment2 PRIVATE ${IGRAPH_LIBRARY_DIRS})
endif()

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
