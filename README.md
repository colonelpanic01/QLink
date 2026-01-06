# Qlink - An Interactive Mental Model Simulator with Link Prediction

AI powered mental model visualization with graph based link prediction using Qt6 and igraph.

## Project Structure

```
├── code/           # Source code
│   ├── core/       # Core logic (models, AI, persistence, nlp)
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
cmake ..
make -j8
./bin/Qlink                    # Run application
./bin/tests/qlink_tests        # Run all tests
```

See [docs/INSTALL.md](docs/INSTALL.md) for the installation instructions

## Major Features Implemented in this Deliverable

### 1. Command Pattern with Full Undo/Redo Support
**Location**: `core/nlp/Commands.h` and `core/nlp/Commands.cpp`

- **AddConceptCommand**: Stores added concept ID for undo
- **RemoveConceptCommand**: Stores removed concept AND all its relationships for complete restoration
- **CreateRelationshipCommand**: Stores relationship ID for undo
- **DeleteRelationshipCommand**: Stores entire relationship object for restoration

All commands now fully implement:
- `execute()`: Performs the operation and stores state
- `undo()`: Reverses the operation using stored state
- `getDescription()`: Returns human-readable description

**Tests**: 24 comprehensive tests in `tests/nlp/test_Commands.cpp` (all passing)

### 2. Natural Language Command Factory
**Location**: `core/nlp/CommandFactory.h` and `core/nlp/CommandFactory.cpp`

Implemented sophisticated regex-based NLP parser with 5 patterns:
1. **Add/Create Concept**: `"add concept AI with description 'Artificial Intelligence'"`
2. **Remove/Delete Concept**: `"remove concept AI"`
3. **Connect Concepts**: `"connect AI to ML with type 'related' directed"`
4. **Disconnect Concepts**: `"disconnect AI from ML"`
5. **Simple Add**: `"add AI"` (shorthand)

Features:
- Case-insensitive matching
- Support for quoted names and descriptions
- Optional parameters (description, type, directed flag)
- Concept name resolution (finds by name, not just ID)
- Helper functions: `trim()`, `toLower()`, `findConceptIdByName()`

**Tests**: 28 comprehensive tests in `tests/nlp/test_CommandFactory.cpp` (all passing)

### 3. Modern UI Design
**Location**: `ui/MainWindow.cpp` - `applyModernStyling()`

Applied comprehensive Qt stylesheet with:
- **Dark menu bar** (#2c3e50) with hover effects
- **Modern toolbar** (#34495e) with rounded buttons
- **Professional color scheme**: Blues (#3498db), grays (#ecf0f1, #bdc3c7)
- **Rounded corners** (border-radius: 4-6px) on all interactive elements
- **Hover states** for better user feedback
- **Smooth transitions** and visual depth
- **Dock widget styling** with custom title bars

### 4. Natural Language Command Panel
**Location**: `ui/MainWindow.cpp` - `setupNaturalLanguagePanel()`

New QDockWidget added to bottom of main window with:
- **Command input**: Multi-line QTextEdit for natural language commands
- **Help text**: Examples of supported commands
- **Execute button**: Styled with primary blue color (#3498db)
- **Clear History button**: For cleaning command log
- **Command history**: QListWidget showing:
  - ✓/✗ status indicators with color coding (green/red)
  - Timestamp for each command
  - Original command text
  - Success/error messages
  - Auto-scrolling (newest first)
  - Limited to 50 entries

Features:
- Real-time validation using `CommandFactory::isValidCommand()`
- Detailed error messages with command examples
- Automatic input clearing on success
- Integration with model change signals
- Professional styling matching overall UI theme

### 6. Test Infrastructure Improvements
**Location**: `tests/CMakeLists.txt`

Simplified test build configuration:
- Removed `gtest_discover_tests()` (no CTest logs)
- Removed gmock dependencies (not needed)
- Changed to `GLOB_RECURSE` for automatic test discovery
- Updated library linking: `Assignment2Core` → `QlinkCore`

## Test Results

**All 158 tests passing** across 8 test suites:
- 22 Concept tests
- 27 Relationship tests
- 27 MentalModel tests
- 11 CommonNeighborPredictor tests
- 9 JaccardCoefficientPredictor tests
- 10 PreferentialAttachmentPredictor tests
- 28 CommandFactory tests
- 24 Commands tests

## Design Patterns Used

1. **Command Pattern**: Execute/undo with state preservation
2. **Factory Pattern**: CommandFactory for creating commands from strings
3. **Strategy Pattern**: Link prediction algorithms
4. **Observer Pattern**: Qt signals/slots for model changes
5. **Singleton-like**: Model management through unique_ptr

## User Experience Improvements

### Before
- Basic UI with default Qt styling
- No natural language input
- Manual menu navigation for all operations
- Limited visual feedback

### After
- Modern, professional appearance with rounded corners and shadows
- Natural language command panel with history
- Real-time command validation and feedback
- Color-coded success/error indicators
- Comprehensive help text and examples
- Auto-scrolling command history
- Dark menu bar and toolbar for better contrast

## Command Examples

Users can now type natural language commands:
```
add concept AI with description 'Artificial Intelligence'
add concept ML
connect AI to ML with type 'related'
connect ML to DL directed
remove concept AI
disconnect ML from DL
```


## Documentation

- [Installation Guide](docs/INSTALL.md) - Setup instructions for different platforms

## Development

Built with:
- **Qt6** for cross-platform GUI
- **igraph** for graph algorithms
- **C++17** with modern practices
- **CMake** for build system
