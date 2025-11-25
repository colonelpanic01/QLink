# Assignment2 - Mental Model Visualization

AI powered mental model visualization with graph based link prediction using Qt6 and igraph.

## Project Structure

```
├── code/           # Source code
│   ├── core/       # Core logic (models, AI, persistence)
│   ├── ui/         # User interface components
│   ├── tests/      # Test files
│   └── main.cpp    # Application entry point
├── docs/           # Documentation
└── README.md       # This file
```

## Quick Start

```bash
# Install dependencies (macOS)
brew install qt6 igraph cmake

# Build and run
cd code
mkdir build && cd build
cmake .. && make
./bin/assignment2
```

See [docs/INSTALL.md](docs/INSTALL.md) for the installation instructions

## Features

- **Interactive Graph Visualization**: Drag and drop mental model editor
- **AI Link Prediction**: Three igraph-powered algorithms:
  - Common Neighbors
  - Jaccard Coefficient  
  - Preferential Attachment
- **AI Assistant**: Added cohere to provide concept descriptions, suggestions, and relationship context
- **JSON Persistence**: Save and load mental models
- **Clean Architecture**: Modular design w/ separation of concerns

## Documentation

- [Installation Guide](docs/INSTALL.md) - Setup instructions for different platforms

## Development

Built with:
- **Qt6** for cross-platform GUI
- **igraph** for graph algorithms
- **C++17** with modern practices
- **CMake** for build system