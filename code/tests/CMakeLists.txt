# Test executable configuration
cmake_minimum_required(VERSION 3.16)

# Collect all test source files
file(GLOB_RECURSE TEST_SOURCES
    test_main.cpp
    model/*.cpp
    ai/*.cpp
    nlp/*.cpp
)

# Create test executable
add_executable(qlink_tests ${TEST_SOURCES})

# Workaround for deprecated AGL framework on newer macOS
if(APPLE)
    set_target_properties(qlink_tests PROPERTIES
        LINK_FLAGS "-Wl,-undefined,dynamic_lookup"
    )
endif()

# Link against GoogleTest and our core library
target_link_libraries(qlink_tests
    PRIVATE
    QlinkCore
    gtest
    gtest_main
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

# Set output directory
set_target_properties(qlink_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
)
