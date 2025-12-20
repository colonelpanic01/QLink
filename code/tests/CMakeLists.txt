# Test executable configuration
cmake_minimum_required(VERSION 3.16)

# Collect all test source files
file(GLOB TEST_SOURCES
    test_main.cpp
    model/*.cpp
    ai/*.cpp
)

# Create test executable
add_executable(qlink_tests ${TEST_SOURCES})

# Link against GoogleTest, GMock, and our core library
target_link_libraries(qlink_tests
    PRIVATE
    Assignment2Core
    gtest
    gtest_main
    gmock
    gmock_main
    Qt6::Core
    Qt6::Network
    ${IGRAPH_LIBRARIES}
)

# Include directories
target_include_directories(qlink_tests
    PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/core
    ${IGRAPH_INCLUDE_DIRS}
)

# Link directories for igraph
if(IGRAPH_LIBRARY_DIRS)
    target_link_directories(qlink_tests PRIVATE ${IGRAPH_LIBRARY_DIRS})
endif()

# Discover tests for CTest
include(GoogleTest)
gtest_discover_tests(qlink_tests)

# Set output directory
set_target_properties(qlink_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
)
