#pragma once
#include <QMainWindow>
#include <memory>
#include <vector>
#include "../core/model/MentalModel.h"
#include "../core/common/DataStructures.h"

class QAction;
class QLabel;
class QProgressBar;
class QTextEdit;
class QLineEdit;
class QPushButton;
class QListWidget;
class QSplitter;


namespace qlink {

class GraphWidget;
class SuggestionPanel;
class ICommand;

/**
 * Main application window
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // File operations
    void newModel();
    void openModel();
    void saveModel();
    void saveAsModel();
    void exportModel();

    // Edit operations
    void addConcept();
    void addRelationship();
    void deleteSelected();
    void undo();
    void redo();

    // Tools operations
    void validateModel();
    void generateSuggestions();
    void showStatistics();

    // Help operations
    void showHelp();
    void showAbout();

    // Natural language command handling
    void executeNaturalLanguageCommand();
    void clearCommandHistory();

    // Model change handling
    void onModelChanged(const ModelChangeEvent& event);
    void updateStatusBar();

private:
    void setupUI();
    void setupNaturalLanguagePanel();
    void applyModernStyling();
    void setupMenus();
    void setupToolbar();
    void setupStatusBar();
    void setupConnections();
    void updateWindowTitle();
    void setModelModified(bool modified = true);
    void connectModelSignals();
    void addCommandToHistory(const QString& command, bool success, const QString& message);
    void executeCommand(std::shared_ptr<ICommand> command);
    void updateUndoRedoActions();

    // Core components
    std::unique_ptr<MentalModel> mentalModel;
    GraphWidget* graphWidget;
    SuggestionPanel* suggestionPanel;
    QSplitter* splitter;
    
    // Natural language components
    QTextEdit* commandInput;
    QPushButton* executeButton;
    QPushButton* clearHistoryButton;
    QListWidget* commandHistory;

    // File management
    QString currentFilePath;
    bool modelModified;
    
    // Command history for undo/redo
    std::vector<std::shared_ptr<ICommand>> undoRedoHistory;
    int undoRedoHistoryIndex; // -1 means no commands, points to last executed command

    // Menu actions
    QAction* newAction;
    QAction* openAction;
    QAction* saveAction;
    QAction* saveAsAction;
    QAction* exportAction;
    QAction* exitAction;
    QAction* undoAction;
    QAction* redoAction;
    QAction* addConceptAction;
    QAction* addRelationshipAction;
    QAction* deleteAction;
    QAction* zoomInAction;
    QAction* zoomOutAction;
    QAction* resetZoomAction;
    QAction* fitToWindowAction;
    QAction* validateModelAction;
    QAction* generateSuggestionsAction;
    QAction* statisticsAction;
    QAction* helpAction;
    QAction* aboutAction;

    // Status bar widgets
    QLabel* conceptCountLabel;
    QLabel* relationshipCountLabel;
    QProgressBar* progressBar;
};

} // namespace qlink