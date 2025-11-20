cmake_minimum_required(VERSION 3.16)
project(Assignment2 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt's MOC
set(CMAKE_AUTOMOC ON)

# Find dependencies
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

# Find igraph with cross-platform support
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(IGRAPH igraph)
endif()

if(NOT IGRAPH_FOUND)
    find_path(IGRAPH_INCLUDE_DIR igraph/igraph.h 
        PATHS /usr/include /usr/local/include /opt/homebrew/include)
    find_library(IGRAPH_LIBRARY igraph 
        PATHS /usr/lib /usr/local/lib /opt/homebrew/lib)
    
    if(IGRAPH_INCLUDE_DIR AND IGRAPH_LIBRARY)
        set(IGRAPH_INCLUDE_DIRS ${IGRAPH_INCLUDE_DIR})
        set(IGRAPH_LIBRARIES ${IGRAPH_LIBRARY})
    else()
        message(FATAL_ERROR "igraph library not found. Please install igraph.")
    endif()
endif()

# Source files
file(GLOB_RECURSE CORE_SOURCES "core/*.cpp")
file(GLOB_RECURSE CORE_HEADERS "core/*.h")
file(GLOB_RECURSE UI_SOURCES "ui/*.cpp")
file(GLOB_RECURSE UI_HEADERS "ui/*.h")

# Core library
add_library(Assignment2Core STATIC ${CORE_SOURCES} ${CORE_HEADERS})
target_link_libraries(Assignment2Core 
    Qt6::Core 
    Qt6::Network 
    ${IGRAPH_LIBRARIES}
)
target_include_directories(Assignment2Core 
    PUBLIC ${CMAKE_SOURCE_DIR}
    PRIVATE ${IGRAPH_INCLUDE_DIRS}
)
if(IGRAPH_LIBRARY_DIRS)
    target_link_directories(Assignment2Core PRIVATE ${IGRAPH_LIBRARY_DIRS})
endif()

# Main executable
add_executable(assignment2 main.cpp ${UI_SOURCES} ${UI_HEADERS})
target_link_libraries(assignment2 
    Assignment2Core
    Qt6::Widgets
    ${IGRAPH_LIBRARIES}
)
if(IGRAPH_LIBRARY_DIRS)
    target_link_directories(assignment2 PRIVATE ${IGRAPH_LIBRARY_DIRS})
endif()

# Output directories
set_target_properties(assignment2 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_target_properties(Assignment2Core PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)
