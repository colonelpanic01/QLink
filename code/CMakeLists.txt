cmake_minimum_required(VERSION 3.16)
project(Qlink VERSION 3.0.0 LANGUAGES CXX)

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
add_library(QlinkCore STATIC ${CORE_SOURCES} ${CORE_HEADERS})
target_link_libraries(QlinkCore 
    Qt6::Core 
    Qt6::Network 
    ${IGRAPH_LIBRARIES}
)
target_include_directories(QlinkCore 
    PUBLIC ${CMAKE_SOURCE_DIR}
    PRIVATE ${IGRAPH_INCLUDE_DIRS}
)
if(IGRAPH_LIBRARY_DIRS)
    target_link_directories(QlinkCore PRIVATE ${IGRAPH_LIBRARY_DIRS})
endif()

# Main executable
qt6_add_resources(RESOURCE_FILES resources.qrc)
add_executable(Qlink main.cpp ${UI_SOURCES} ${UI_HEADERS} ${RESOURCE_FILES})

# Workaround for deprecated AGL framework on newer macOS
if(APPLE)
    set_target_properties(Qlink PROPERTIES
        LINK_FLAGS "-Wl,-undefined,dynamic_lookup"
    )
endif()

target_link_libraries(Qlink 
    QlinkCore
    Qt6::Widgets
    ${IGRAPH_LIBRARIES}
)
if(IGRAPH_LIBRARY_DIRS)
    target_link_directories(Qlink PRIVATE ${IGRAPH_LIBRARY_DIRS})
endif()

# Enable testing
enable_testing()

# Force use of FetchContent for GoogleTest to ensure ARM compatibility
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add tests subdirectory
add_subdirectory(tests)

# Output directories
set_target_properties(Qlink PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_target_properties(QlinkCore PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)
