[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/dL5Ec4iv)
[![Work in MakeCode](https://classroom.github.com/assets/work-in-make-code-8824cc13a1a3f34ffcd245c82f0ae96fdae6b7d554b6539aec3a03a70825519c.svg)](https://classroom.github.com/online_ide?assignment_repo_id=22091199&assignment_repo_type=AssignmentRepo)
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
