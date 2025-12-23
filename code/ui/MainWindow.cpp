#include "MainWindow.h"
#include "GraphWidget.h"
#include "SuggestionPanel.h"
#include "../core/persistence/ModelManager.h"
#include "../core/nlp/CommandFactory.h"
#include <QDebug>
#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QAction>
#include <QKeySequence>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QDockWidget>
#include <QProgressBar>
#include <QTimer>
#include <QFileInfo>
#include <QStandardPaths>
#include <QInputDialog>
#include <QDateTime>

namespace qlink {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), mentalModel(std::make_unique<MentalModel>("New Model")), 
      graphWidget(nullptr), suggestionPanel(nullptr), 
      commandInput(nullptr), executeButton(nullptr), clearHistoryButton(nullptr), commandHistory(nullptr),
      modelModified(false) {
    try {
        setupUI();
        setupNaturalLanguagePanel();
        applyModernStyling();
        setupMenus();
        setupToolbar();
        setupStatusBar();
        setupConnections();
        updateWindowTitle();
        updateStatusBar();
        

    } catch (const std::exception& e) {
        qDebug() << "Error in MainWindow constructor:" << e.what();
        throw;
    }
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI() {
    // Set window properties
    setMinimumSize(1200, 800);
    resize(1400, 900);
    
    // Create central widget with splitter
    auto centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    auto mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    splitter = new QSplitter(Qt::Horizontal, this);
    mainLayout->addWidget(splitter, 1);

    // Create graph widget
    graphWidget = new GraphWidget(this);
    graphWidget->setModel(mentalModel.get());

    // Create suggestion panel
    suggestionPanel = new SuggestionPanel(this);
    suggestionPanel->setMinimumWidth(280);

    // Add widgets to splitter
    splitter->addWidget(graphWidget);
    splitter->addWidget(suggestionPanel);

    // Set initial sizes (graph takes 70%, suggestions take 30%)
    splitter->setSizes({700, 300});
    
    // Make splitter handle more visible
    splitter->setHandleWidth(2);
}

void MainWindow::setupNaturalLanguagePanel() {
    // Create regular widget for natural language commands (no dock)
    auto nlWidget = new QWidget(this);
    nlWidget->setMaximumHeight(180);
    auto nlLayout = new QVBoxLayout(nlWidget);
    nlLayout->setSpacing(6);
    nlLayout->setContentsMargins(8, 8, 8, 8);
    
    // Help text only
    auto helpText = new QLabel("Enter commands like: 'add concept AI', 'connect AI to ML', 'remove concept X'");
    helpText->setWordWrap(true);
    nlLayout->addWidget(helpText);
    
    // Command input area
    commandInput = new QTextEdit();
    commandInput->setPlaceholderText("Type your natural language command here...");
    commandInput->setMaximumHeight(60);
    commandInput->setMinimumHeight(40);
    nlLayout->addWidget(commandInput);
    
    // Buttons
    auto buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    
    executeButton = new QPushButton("Execute Command");
    buttonLayout->addWidget(executeButton);
    
    clearHistoryButton = new QPushButton("Clear History");
    buttonLayout->addWidget(clearHistoryButton);
    buttonLayout->addStretch();
    
    nlLayout->addLayout(buttonLayout);
    
    // Command history
    commandHistory = new QListWidget();
    commandHistory->setMaximumHeight(100);
    nlLayout->addWidget(commandHistory);
    
    // Add natural language panel to central widget's main layout
    auto centralWidget = qobject_cast<QWidget*>(this->centralWidget());
    if (centralWidget) {
        auto mainLayout = qobject_cast<QVBoxLayout*>(centralWidget->layout());
        if (mainLayout) {
            mainLayout->addWidget(nlWidget, 0);
        }
    }
    
    // Connect signals
    connect(executeButton, &QPushButton::clicked, this, &MainWindow::executeNaturalLanguageCommand);
    connect(clearHistoryButton, &QPushButton::clicked, this, &MainWindow::clearCommandHistory);
    connect(commandInput, &QTextEdit::textChanged, [this]() {
        executeButton->setEnabled(!commandInput->toPlainText().trimmed().isEmpty());
    });
    
    executeButton->setEnabled(false);
}

void MainWindow::applyModernStyling() {
    // No custom styling - using Qt's default or system theme
}

void MainWindow::setupMenus() {
    // File Menu
    auto fileMenu = menuBar()->addMenu("&File");

    newAction = new QAction("&New Model", this);
    newAction->setShortcut(QKeySequence::New);
    newAction->setStatusTip("Create a new mental model");
    fileMenu->addAction(newAction);

    openAction = new QAction("&Open...", this);
    openAction->setShortcut(QKeySequence::Open);
    openAction->setStatusTip("Open an existing mental model");
    fileMenu->addAction(openAction);

    saveAction = new QAction("&Save", this);
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setStatusTip("Save the current mental model");
    fileMenu->addAction(saveAction);

    saveAsAction = new QAction("Save &As...", this);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    saveAsAction->setStatusTip("Save the mental model with a new name");
    fileMenu->addAction(saveAsAction);

    fileMenu->addSeparator();

    exportAction = new QAction("&Export...", this);
    exportAction->setStatusTip("Export model to various formats");
    fileMenu->addAction(exportAction);

    fileMenu->addSeparator();

    exitAction = new QAction("E&xit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    exitAction->setStatusTip("Exit the application");
    fileMenu->addAction(exitAction);

    // Edit Menu
    auto editMenu = menuBar()->addMenu("&Edit");

    undoAction = new QAction("&Undo", this);
    undoAction->setShortcut(QKeySequence::Undo);
    undoAction->setStatusTip("Undo the last action");
    undoAction->setEnabled(false); // TODO: Implement undo system
    editMenu->addAction(undoAction);

    redoAction = new QAction("&Redo", this);
    redoAction->setShortcut(QKeySequence::Redo);
    redoAction->setStatusTip("Redo the last undone action");
    redoAction->setEnabled(false); // TODO: Implement redo system
    editMenu->addAction(redoAction);

    editMenu->addSeparator();

    addConceptAction = new QAction("Add &Concept", this);
    addConceptAction->setShortcut(QKeySequence("Ctrl+Shift+C"));
    addConceptAction->setStatusTip("Add a new concept to the model");
    editMenu->addAction(addConceptAction);

    addRelationshipAction = new QAction("Add &Relationship", this);
    addRelationshipAction->setShortcut(QKeySequence("Ctrl+Shift+R"));
    addRelationshipAction->setStatusTip("Add a new relationship between concepts");
    editMenu->addAction(addRelationshipAction);

    editMenu->addSeparator();

    deleteAction = new QAction("&Delete Selected", this);
    deleteAction->setShortcut(QKeySequence::Delete);
    deleteAction->setStatusTip("Delete the selected concept or relationship");
    editMenu->addAction(deleteAction);

    // View Menu
    auto viewMenu = menuBar()->addMenu("&View");

    zoomInAction = new QAction("Zoom &In", this);
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    zoomInAction->setStatusTip("Zoom in on the graph");
    viewMenu->addAction(zoomInAction);

    zoomOutAction = new QAction("Zoom &Out", this);
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    zoomOutAction->setStatusTip("Zoom out on the graph");
    viewMenu->addAction(zoomOutAction);

    resetZoomAction = new QAction("&Reset Zoom", this);
    resetZoomAction->setShortcut(QKeySequence("Ctrl+0"));
    resetZoomAction->setStatusTip("Reset zoom to 100%");
    viewMenu->addAction(resetZoomAction);

    viewMenu->addSeparator();

    fitToWindowAction = new QAction("&Fit to Window", this);
    fitToWindowAction->setShortcut(QKeySequence("Ctrl+F"));
    fitToWindowAction->setStatusTip("Fit the entire graph in the window");
    viewMenu->addAction(fitToWindowAction);

    // Tools Menu
    auto toolsMenu = menuBar()->addMenu("&Tools");

    validateModelAction = new QAction("&Validate Model", this);
    validateModelAction->setStatusTip("Check the model for consistency and errors");
    toolsMenu->addAction(validateModelAction);

    generateSuggestionsAction = new QAction("Generate &Suggestions", this);
    generateSuggestionsAction->setShortcut(QKeySequence("F5"));
    generateSuggestionsAction->setStatusTip("Generate AI-powered link suggestions");
    toolsMenu->addAction(generateSuggestionsAction);

    toolsMenu->addSeparator();

    statisticsAction = new QAction("Model &Statistics", this);
    statisticsAction->setStatusTip("Show detailed model statistics");
    toolsMenu->addAction(statisticsAction);

    // Help Menu
    auto helpMenu = menuBar()->addMenu("&Help");

    helpAction = new QAction("&Help", this);
    helpAction->setShortcut(QKeySequence::HelpContents);
    helpAction->setStatusTip("Show help documentation");
    helpMenu->addAction(helpAction);

    aboutAction = new QAction("&About", this);
    aboutAction->setStatusTip("Show information about this application");
    helpMenu->addAction(aboutAction);
}

void MainWindow::setupToolbar() {
    auto toolbar = addToolBar("Main");

    // File operations
    toolbar->addAction(newAction);
    toolbar->addAction(openAction);
    toolbar->addAction(saveAction);
    toolbar->addSeparator();

    // Edit operations
    toolbar->addAction(addConceptAction);
    toolbar->addAction(addRelationshipAction);
    toolbar->addAction(deleteAction);
    toolbar->addSeparator();

    // View operations
    toolbar->addAction(zoomInAction);
    toolbar->addAction(zoomOutAction);
    toolbar->addAction(resetZoomAction);
    toolbar->addAction(fitToWindowAction);
    toolbar->addSeparator();

    // Tools
    toolbar->addAction(generateSuggestionsAction);
}

void MainWindow::setupStatusBar() {
    // Model statistics labels
    conceptCountLabel = new QLabel("Concepts: 0");
    relationshipCountLabel = new QLabel("Relationships: 0");

    // Progress bar for long operations
    progressBar = new QProgressBar();
    progressBar->setVisible(false);

    statusBar()->addWidget(conceptCountLabel);
    statusBar()->addWidget(relationshipCountLabel);
    statusBar()->addPermanentWidget(progressBar);

    // General status message
    statusBar()->showMessage("Ready");
}

void MainWindow::setupConnections() {
    // File menu connections
    connect(newAction, &QAction::triggered, this, &MainWindow::newModel);
    connect(openAction, &QAction::triggered, this, &MainWindow::openModel);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveModel);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveAsModel);
    connect(exportAction, &QAction::triggered, this, &MainWindow::exportModel);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    // Edit menu connections
    connect(addConceptAction, &QAction::triggered, this, &MainWindow::addConcept);
    connect(addRelationshipAction, &QAction::triggered, this, &MainWindow::addRelationship);
    connect(deleteAction, &QAction::triggered, this, &MainWindow::deleteSelected);

    // View menu connections (only if graphWidget exists)
    if (graphWidget) {
        connect(zoomInAction, &QAction::triggered, graphWidget, &GraphWidget::zoomIn);
        connect(zoomOutAction, &QAction::triggered, graphWidget, &GraphWidget::zoomOut);
        connect(resetZoomAction, &QAction::triggered, graphWidget, &GraphWidget::resetZoom);
        connect(fitToWindowAction, &QAction::triggered, graphWidget, &GraphWidget::fitToWindow);
    }

    // Tools menu connections
    connect(validateModelAction, &QAction::triggered, this, &MainWindow::validateModel);
    connect(generateSuggestionsAction, &QAction::triggered, this, &MainWindow::generateSuggestions);
    connect(statisticsAction, &QAction::triggered, this, &MainWindow::showStatistics);

    // Help menu connections
    connect(helpAction, &QAction::triggered, this, &MainWindow::showHelp);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);

    // Connect model signals
    connectModelSignals();
    
    // Connect suggestion panel (only if it exists)
    if (suggestionPanel) {
        connect(suggestionPanel, &SuggestionPanel::suggestionAccepted, 
                this, [this](const LinkSuggestion& suggestion) {
                    statusBar()->showMessage("Suggestion accepted and relationship created", 2000);
                });
        connect(suggestionPanel, &SuggestionPanel::suggestionRejected,
                this, [this](const LinkSuggestion& suggestion) {
                    statusBar()->showMessage("Suggestion rejected", 2000);
                });
    }
}

void MainWindow::setModelModified(bool modified) {
    if (modelModified != modified) {
        modelModified = modified;
        updateWindowTitle();
    }
}

// Slot implementations
void MainWindow::newModel() {
    // Check if current model needs saving
    if (modelModified && !currentFilePath.isEmpty()) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, 
            "Unsaved Changes", 
            "The current model has unsaved changes. Do you want to save before creating a new model?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        
        if (reply == QMessageBox::Save) {
            saveModel();
        } else if (reply == QMessageBox::Cancel) {
            return; // User cancelled, don't create new model
        }
    }
    
    // Disconnect widgets from old model before destroying it
    if (mentalModel) {
        disconnect(mentalModel.get(), nullptr, this, nullptr);
        if (graphWidget) {
            graphWidget->setModel(nullptr);  // Clear old model first
        }
        if (suggestionPanel) {
            suggestionPanel->setModel(nullptr);  // Clear old model first
        }
    }
    
    // Create new model
    mentalModel = std::make_unique<MentalModel>("New Model");
    
    // Set new model in widgets
    if (graphWidget) {
        graphWidget->setModel(mentalModel.get());
    }
    if (suggestionPanel) {
        suggestionPanel->setModel(mentalModel.get());
    }
    
    // Reconnect signals to new model
    connectModelSignals();
    
    currentFilePath.clear();
    setModelModified(false);
    updateWindowTitle();
    updateStatusBar();
    statusBar()->showMessage("New model created", 2000);
}

void MainWindow::openModel() {
    QString fileName = QFileDialog::getOpenFileName(this,
        "Open Mental Model", "", "JSON Files (*.json)");
    if (!fileName.isEmpty()) {
        ModelManager manager;
        auto loadedModel = manager.loadModel(fileName);
        if (loadedModel) {
            // Disconnect widgets from old model before destroying it
            if (mentalModel) {
                disconnect(mentalModel.get(), nullptr, this, nullptr);
                if (graphWidget) {
                    graphWidget->setModel(nullptr);  // Clear old model first
                }
                if (suggestionPanel) {
                    suggestionPanel->setModel(nullptr);  // Clear old model first
                }
            }
            
            // Create new model
            mentalModel = std::move(loadedModel);
            
            // Set new model in widgets
            if (graphWidget) {
                graphWidget->setModel(mentalModel.get());
            }
            if (suggestionPanel) {
                suggestionPanel->setModel(mentalModel.get());
            }
            
            // Reconnect signals to new model
            connectModelSignals();
            
            currentFilePath = fileName;
            setModelModified(false);
            updateWindowTitle();
            updateStatusBar();
            statusBar()->showMessage("Model loaded successfully: " + QFileInfo(fileName).baseName(), 2000);
        } else {
            QMessageBox::warning(this, "Load Error", 
                QString("Failed to load model from file: %1").arg(fileName));
        }
    }
}

void MainWindow::saveModel() {
    if (currentFilePath.isEmpty()) {
        saveAsModel();
    } else {
        ModelManager manager;
        if (manager.saveModel(*mentalModel, currentFilePath)) {
            setModelModified(false);
            statusBar()->showMessage("Model saved: " + QFileInfo(currentFilePath).baseName(), 2000);
        } else {
            QMessageBox::warning(this, "Save Error", 
                QString("Failed to save model: Check file permissions and disk space."));
        }
    }
}

void MainWindow::saveAsModel() {
    QString fileName = QFileDialog::getSaveFileName(this,
        "Save Mental Model", "", "JSON Files (*.json)");
    if (!fileName.isEmpty()) {
        ModelManager manager;
        if (manager.saveModel(*mentalModel, fileName)) {
            currentFilePath = fileName;
            setModelModified(false);
            updateWindowTitle();
            statusBar()->showMessage("Model saved as: " + QFileInfo(fileName).baseName(), 2000);
        } else {
            QMessageBox::warning(this, "Save Error", 
                QString("Failed to save model: Check file permissions and disk space."));
        }
    }
}

void MainWindow::exportModel() {
    QString fileName = QFileDialog::getSaveFileName(this,
        "Export Mental Model", "", 
        "JSON Files (*.json)");
    if (!fileName.isEmpty()) {
        ModelManager manager;
        bool success = manager.exportModel(*mentalModel, fileName, ExportFormat::JSON);
        
        if (success) {
            QFileInfo fileInfo(fileName);
            statusBar()->showMessage("Exported to: " + fileInfo.baseName(), 2000);
        } else {
            QMessageBox::warning(this, "Export Error", 
                QString("Failed to export model: Check file permissions and disk space."));
        }
    }
}

void MainWindow::addConcept() {
    bool ok;
    QString name = QInputDialog::getText(this, "Add Concept", 
                                        "Concept name:", QLineEdit::Normal, 
                                        "", &ok);
    
    if (ok && !name.isEmpty()) {
        // Create the concept
        auto concept = std::make_unique<Concept>(name.toStdString());
        
        // Add to model
        mentalModel->addConcept(std::move(concept));
        
        // Mark as modified
        setModelModified(true);
        
        statusBar()->showMessage(QString("Added concept: %1").arg(name), 2000);
    }
}

void MainWindow::addRelationship() {
    // Get all concepts for selection
    const auto& concepts = mentalModel->getConcepts();
    
    if (concepts.size() < 2) {
        QMessageBox::information(this, "Add Relationship", 
                                "You need at least 2 concepts before adding relationships.");
        return;
    }
    
    // Create dialog to select source and target concepts
    QStringList conceptNames;
    for (const auto& concept : concepts) {
        conceptNames << QString::fromStdString(concept->getName());
    }
    
    bool ok;
    QString sourceName = QInputDialog::getItem(this, "Add Relationship", 
                                              "Source concept:", conceptNames, 
                                              0, false, &ok);
    if (!ok) return;
    
    QString targetName = QInputDialog::getItem(this, "Add Relationship", 
                                              "Target concept:", conceptNames, 
                                              0, false, &ok);
    if (!ok) return;
    
    QString relationshipType = QInputDialog::getText(this, "Add Relationship",
                                                    "Relationship type:", QLineEdit::Normal,
                                                    "relates to", &ok);
    if (!ok || relationshipType.isEmpty()) return;
    
    // Find the concept IDs
    std::string sourceId, targetId;
    for (const auto& concept : concepts) {
        if (concept->getName() == sourceName.toStdString()) {
            sourceId = concept->getId();
        }
        if (concept->getName() == targetName.toStdString()) {
            targetId = concept->getId();
        }
    }
    
    if (!sourceId.empty() && !targetId.empty() && sourceId != targetId) {
        // Use the 3-parameter constructor (sourceId, targetId, type, directed, weight)
        auto relationship = std::make_unique<Relationship>(sourceId, targetId, relationshipType.toStdString(), false, 1.0);
        mentalModel->addRelationship(std::move(relationship));
        setModelModified(true);
        statusBar()->showMessage(QString("Added relationship: %1 %2 %3")
                                .arg(sourceName, relationshipType, targetName), 3000);
    } else if (sourceId == targetId) {
        QMessageBox::warning(this, "Add Relationship", "Source and target cannot be the same concept.");
    }
}

void MainWindow::deleteSelected() {
    if (!graphWidget || !mentalModel) return;
    
    // Get selected items from graph widget's scene
    QGraphicsScene* graphScene = static_cast<QGraphicsView*>(graphWidget)->scene();
    if (!graphScene) return;
    
    auto selectedItems = graphScene->selectedItems();
    
    if (selectedItems.isEmpty()) {
        statusBar()->showMessage("No items selected", 2000);
        return;
    }
    
    // Ask for confirmation
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Delete Selected",
        QString("Are you sure you want to delete %1 selected item(s)?").arg(selectedItems.size()),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply != QMessageBox::Yes) {
        return;
    }
    
    // Delete each selected item from the model (which will update the view)
    int deletedCount = 0;
    for (auto item : selectedItems) {
        // Check if it's a concept
        if (auto conceptItem = dynamic_cast<ConceptGraphicsItem*>(item)) {
            const Concept* concept = conceptItem->getConcept();
            if (concept) {
                mentalModel->removeConcept(concept->getId());
                deletedCount++;
            }
        }
        // Check if it's a relationship
        else if (auto relationshipItem = dynamic_cast<RelationshipGraphicsItem*>(item)) {
            const Relationship* relationship = relationshipItem->getRelationship();
            if (relationship) {
                mentalModel->removeRelationship(relationship->getId());
                deletedCount++;
            }
        }
    }
    
    setModelModified();
    statusBar()->showMessage(QString("Deleted %1 item(s)").arg(deletedCount), 2000);
}

void MainWindow::validateModel() {
    if (mentalModel->isValid()) {
        QMessageBox::information(this, "Model Validation", 
            "The model is valid and consistent.");
    } else {
        auto errors = mentalModel->getValidationErrors();
        QString errorText = "Model validation failed:\n\n";
        for (const auto& error : errors) {
            errorText += "• " + QString::fromStdString(error) + "\n";
        }
        QMessageBox::warning(this, "Model Validation", errorText);
    }
}

void MainWindow::generateSuggestions() {
    progressBar->setVisible(true);
    progressBar->setRange(0, 0); // Indeterminate progress
    statusBar()->showMessage("Generating concept suggestions...");

    // Trigger suggestion generation in the panel
    suggestionPanel->generateSuggestions();

    // Hide progress after a delay
    QTimer::singleShot(2000, [this]() {
        progressBar->setVisible(false);
        statusBar()->showMessage("Suggestions generated", 2000);
    });
}

void MainWindow::showStatistics() {
    auto stats = mentalModel->getStatistics();
    QString statsText = QString(
        "Model Statistics:\n\n"
        "Concepts: %1\n"
        "Relationships: %2\n"
        "Orphaned Concepts: %3\n"
        "Average Connections: %4\n"
        "Max Connections: %5\n"
        "Min Connections: %6"
    ).arg(stats.conceptCount)
     .arg(stats.relationshipCount)
     .arg(stats.orphanedConceptCount)
     .arg(stats.averageConnections, 0, 'f', 2)
     .arg(stats.maxConnections)
     .arg(stats.minConnections);

    QMessageBox::information(this, "Model Statistics", statsText);
}

void MainWindow::showHelp() {
    QString helpText = 
        "Assignment2 Mental Model Simulator Help\n\n"
        "Keyboard Shortcuts:\n"
        "• Ctrl+N: New model\n"
        "• Ctrl+O: Open model\n"
        "• Ctrl+S: Save model\n"
        "• Ctrl+Shift+C: Add concept\n"
        "• Ctrl+Shift+R: Add relationship\n"
        "• Delete: Delete selected\n"
        "• Ctrl++: Zoom in\n"
        "• Ctrl+-: Zoom out\n"
        "• Ctrl+0: Reset zoom\n"
        "• Ctrl+F: Fit to window\n"
        "• F5: Generate suggestions\n\n"
        "For more information, visit the project documentation.";

    QMessageBox::information(this, "Help", helpText);
}

void MainWindow::showAbout() {
    QString aboutText = 
        "<h2>QLink v3/h2>"
        "<p><b>Mental Model and Knowledge Management Visualizer!</b></p>"
        "<p>QLink helps you organize, visualize, and understand complex "
        "relationships between concepts via a graph based representations.</p>"
        "<h3>Features:</h3>"
        "<ul>"
        "<li>Interactive graph visualization</li>"
        "<li>AI-powered relationship suggestions</li>"
        "<li>Natural language command interface</li>"
        "<li>Import/Export functionality</li>"
        "<li>Model validation</li>"
        "</ul>"
        "<p><i>Developed as part of CS 3307 - Object-Oriented Design & Analysis</i></p>";

    QMessageBox::about(this, "About QLink", aboutText);
}

void MainWindow::onModelChanged(const ModelChangeEvent& event) {
    updateStatusBar();
    setModelModified(true);
}

void MainWindow::executeNaturalLanguageCommand() {
    QString commandText = commandInput->toPlainText().trimmed();
    if (commandText.isEmpty()) {
        return;
    }
    
    try {
        // Check if command is valid
        if (!CommandFactory::isValidCommand(commandText.toStdString())) {
            addCommandToHistory(commandText, false, "Invalid command format");
            QMessageBox::warning(this, "Invalid Command", 
                "The command could not be understood. Please check your syntax.\n\n"
                "Examples:\n"
                "• add concept AI with description 'Artificial Intelligence'\n"
                "• connect AI to ML with type 'related'\n"
                "• remove concept AI\n"
                "• disconnect AI from ML");
            return;
        }
        
        // Create command
        auto command = CommandFactory::createCommand(commandText.toStdString(), mentalModel.get());
        if (!command) {
            addCommandToHistory(commandText, false, "Failed to create command");
            QMessageBox::warning(this, "Command Error", 
                "Failed to create command. Please check your syntax.");
            return;
        }
        
        // Execute command
        command->execute();
        
        // Success feedback
        addCommandToHistory(commandText, true, "Successfully executed");
        statusBar()->showMessage("Command executed successfully", 3000);
        
        // Clear input
        commandInput->clear();
        
        // Update graph - the model signals will trigger the update
        setModelModified(true);
        
    } catch (const std::exception& e) {
        addCommandToHistory(commandText, false, QString("Error: %1").arg(e.what()));
        QMessageBox::critical(this, "Execution Error", 
            QString("Failed to execute command:\n%1").arg(e.what()));
    }
}

void MainWindow::clearCommandHistory() {
    if (commandHistory->count() > 0) {
        auto reply = QMessageBox::question(this, "Clear History",
            "Are you sure you want to clear the command history?",
            QMessageBox::Yes | QMessageBox::No);
            
        if (reply == QMessageBox::Yes) {
            commandHistory->clear();
            statusBar()->showMessage("Command history cleared", 2000);
        }
    }
}

void MainWindow::addCommandToHistory(const QString& command, bool success, const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString status = success ? "✓" : "✗";
    
    // Simple text format without HTML
    QString historyEntry = QString("%1 [%2] %3 - %4")
        .arg(status, timestamp, command, message);
    
    auto item = new QListWidgetItem(historyEntry);
    item->setData(Qt::UserRole, command); // Store original command
    
    // Color code by success/failure
    if (success) {
        item->setForeground(QColor("#27ae60")); // Green for success
    } else {
        item->setForeground(QColor("#e74c3c")); // Red for failure
    }
    
    commandHistory->insertItem(0, item); // Add to top
    
    // Limit history to 50 items
    while (commandHistory->count() > 50) {
        delete commandHistory->takeItem(commandHistory->count() - 1);
    }
}

void MainWindow::updateWindowTitle() {
    QString title = "QLink - " + QString::fromStdString(mentalModel->getModelName());
    if (!currentFilePath.isEmpty()) {
        QFileInfo fileInfo(currentFilePath);
        title = "QLink - " + fileInfo.baseName();
    }
    
    // Add asterisk for unsaved changes
    if (modelModified) {
        title += " *";
    }
    
    setWindowTitle(title);
}

void MainWindow::updateStatusBar() {
    auto stats = mentalModel->getStatistics();
    conceptCountLabel->setText(QString("Concepts: %1").arg(stats.conceptCount));
    relationshipCountLabel->setText(QString("Relationships: %1").arg(stats.relationshipCount));
}

void MainWindow::connectModelSignals() {
    if (!mentalModel) return;
    
    // Model change notifications
    connect(mentalModel.get(), &MentalModel::modelChanged, this, &MainWindow::onModelChanged);
    connect(mentalModel.get(), &MentalModel::conceptAdded, this, [this](const QString&) { updateStatusBar(); });
    connect(mentalModel.get(), &MentalModel::conceptRemoved, this, [this](const QString&) { updateStatusBar(); });
    connect(mentalModel.get(), &MentalModel::relationshipAdded, this, [this](const QString&) { updateStatusBar(); });
    connect(mentalModel.get(), &MentalModel::relationshipRemoved, this, [this](const QString&) { updateStatusBar(); });
}

} // namespace qlink