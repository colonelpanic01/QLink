#!/bin/bash

# Setup and run GoogleTest suite for Qlink

echo "=========================================="
echo "Qlink GoogleTest Setup and Execution"
echo "=========================================="

# Navigate to build directory
cd "$(dirname "$0")/build" || exit 1

echo ""
echo "Step 1: Configuring CMake with testing enabled..."
cmake .. || {
    echo "ERROR: CMake configuration failed!"
    exit 1
}

echo ""
echo "Step 2: Building project and tests..."
make -j$(sysctl -n hw.ncpu) || {
    echo "ERROR: Build failed!"
    exit 1
}

echo ""
echo "Step 3: Running GoogleTest suite..."
echo "=========================================="
./bin/tests/qlink_tests || {
    echo ""
    echo "Some tests failed. Check output above for details."
    exit 1
}

echo ""
echo "=========================================="
echo "✓ All tests passed successfully!"
echo "=========================================="

# Optional: Run with CTest for more detailed output
echo ""
echo "Running CTest for detailed test results..."
ctest --output-on-failure

echo ""
echo "Test execution complete!"
