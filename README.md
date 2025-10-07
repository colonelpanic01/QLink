# OOP Assignment 3 — C++ (CMake + GoogleTest)

## Repository Structure

- `code/` – C++ source code and tests
- `report/` – Project report, UML diagrams, and any design files
- `README.md` – This file

## Build & Test Instructions

To compile and test locally (assuming you're using CMake and GoogleTest):

```bash
# Navigate into a new build folder
mkdir build && cd build

# Generate build files
cmake ../code -DCMAKE_BUILD_TYPE=Release

# Build the project
cmake --build .

# Run tests (if using GoogleTest)
ctest --output-on-failure
