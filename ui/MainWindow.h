#pragma once
#include <QMainWindow>
#include <memory>
#include "../core/model/MentalModel.h"
#include "../core/common/DataStructures.h"

class QAction;
class QLabel;
class QProgressBar;


namespace qlink {

class GraphWidget;
class SuggestionPanel;

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

    // Tools operations
    void validateModel();
    void generateSuggestions();
    void showStatistics();

    // Help operations
    void showHelp();
    void showAbout();

    // Model change handling
    void onModelChanged(const ModelChangeEvent& event);
    void updateStatusBar();

private:
    void setupUI();

    void setupMenus();
    void setupToolbar();
    void setupStatusBar();
    void setupConnections();
    void updateWindowTitle();
    void setModelModified(bool modified = true);
    void connectModelSignals();

    // Core components
    std::unique_ptr<MentalModel> mentalModel;
    GraphWidget* graphWidget;
    SuggestionPanel* suggestionPanel;

    // File management
    QString currentFilePath;
    bool modelModified;

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