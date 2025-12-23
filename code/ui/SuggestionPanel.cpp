#include "SuggestionPanel.h"
#include "../core/ai/CommonNeighborPredictor.h"
#include "../core/ai/JaccardCoefficientPredictor.h"
#include "../core/ai/PreferentialAttachmentPredictor.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QProgressBar>
#include <QComboBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QSplitter>
#include <QTextEdit>
#include <QHeaderView>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QDebug>

namespace qlink {

SuggestionPanel::SuggestionPanel(QWidget *parent)
    : QWidget(parent), model(nullptr) {
    setupUI();
    setupConnections();
}

SuggestionPanel::~SuggestionPanel() = default;

void SuggestionPanel::setupUI() {
    auto mainLayout = new QVBoxLayout(this);
    
    // Controls section
    setupControlsSection(mainLayout);
    
    // Suggestions list - give this the most space
    setupSuggestionsSection(mainLayout);
    
    // Action buttons
    setupActionButtons(mainLayout);
}

void SuggestionPanel::setupControlsSection(QVBoxLayout* mainLayout) {
    auto controlsGroup = new QGroupBox("Controls");
    auto controlsLayout = new QVBoxLayout(controlsGroup);
    
    // Algorithm selection
    auto algorithmLayout = new QHBoxLayout();
    algorithmLayout->addWidget(new QLabel("Algorithm:"));
    algorithmCombo = new QComboBox();
    algorithmCombo->addItem("Common Neighbors", "common_neighbors");
    algorithmCombo->addItem("Jaccard Coefficient", "jaccard");
    algorithmCombo->addItem("Preferential Attachment", "preferential");
    algorithmCombo->addItem("All Algorithms", "all");
    algorithmLayout->addWidget(algorithmCombo);
    controlsLayout->addLayout(algorithmLayout);
    
    // Confidence threshold
    auto thresholdLayout = new QHBoxLayout();
    thresholdLayout->addWidget(new QLabel("Min Confidence:"));
    confidenceThreshold = new QLineEdit("0.5");
    confidenceThreshold->setMinimumWidth(50);
    thresholdLayout->addWidget(confidenceThreshold);
    thresholdLayout->addStretch();
    controlsLayout->addLayout(thresholdLayout);
    
    // Generate button
    generateButton = new QPushButton("Generate Suggestions");
    generateButton->setMinimumHeight(36);
    controlsLayout->addWidget(generateButton);
    
    // Progress bar
    progressBar = new QProgressBar();
    progressBar->setVisible(false);
    controlsLayout->addWidget(progressBar);
    
    mainLayout->addWidget(controlsGroup, 0); // No stretch - fixed size
}

void SuggestionPanel::setupSuggestionsSection(QVBoxLayout* mainLayout) {
    auto suggestionsGroup = new QGroupBox("Suggestions");
    auto suggestionsLayout = new QVBoxLayout(suggestionsGroup);
    
    // Filter controls
    auto filterLayout = new QHBoxLayout();
    filterLayout->addWidget(new QLabel("Filter:"));
    filterEdit = new QLineEdit();
    filterEdit->setPlaceholderText("Search suggestions...");
    filterLayout->addWidget(filterEdit);
    
    sortCombo = new QComboBox();
    sortCombo->addItem("By Confidence", "confidence");
    sortCombo->addItem("By Source", "source");
    sortCombo->addItem("By Target", "target");
    sortCombo->addItem("By Algorithm", "algorithm");
    filterLayout->addWidget(sortCombo);
    suggestionsLayout->addLayout(filterLayout);
    
    // Suggestions tree
    suggestionsTree = new QTreeWidget();
    suggestionsTree->setHeaderLabels({"Source", "Target", "Algorithm", "Confidence"});
    suggestionsTree->setAlternatingRowColors(true);
    suggestionsTree->setSelectionMode(QAbstractItemView::SingleSelection);
    suggestionsTree->setSortingEnabled(true);
    
    // Set columns to resize dynamically
    suggestionsTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    suggestionsTree->header()->setStretchLastSection(true); // Stretch last column to fill space
    suggestionsTree->setWordWrap(false);
    
    suggestionsLayout->addWidget(suggestionsTree);
    mainLayout->addWidget(suggestionsGroup, 4); // Increased stretch factor from 3 to 4
}

void SuggestionPanel::setupActionButtons(QVBoxLayout* mainLayout) {
    auto buttonsLayout = new QHBoxLayout();
    
    acceptButton = new QPushButton("Accept");
    acceptButton->setEnabled(false);
    
    rejectButton = new QPushButton("Reject");
    rejectButton->setEnabled(false);
    
    clearButton = new QPushButton("Clear All");
    
    buttonsLayout->addWidget(acceptButton);
    buttonsLayout->addWidget(rejectButton);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(clearButton);
    
    mainLayout->addLayout(buttonsLayout);
}

void SuggestionPanel::setupConnections() {
    connect(generateButton, &QPushButton::clicked, this, &SuggestionPanel::generateSuggestions);
    connect(acceptButton, &QPushButton::clicked, this, &SuggestionPanel::acceptSuggestion);
    connect(rejectButton, &QPushButton::clicked, this, &SuggestionPanel::rejectSuggestion);
    connect(clearButton, &QPushButton::clicked, this, &SuggestionPanel::clearSuggestions);
    
    connect(suggestionsTree, &QTreeWidget::itemSelectionChanged, 
            this, &SuggestionPanel::onSelectionChanged);
    connect(suggestionsTree, &QTreeWidget::itemDoubleClicked,
            this, &SuggestionPanel::acceptSuggestion);
    
    connect(filterEdit, &QLineEdit::textChanged, this, &SuggestionPanel::filterSuggestions);
    connect(sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SuggestionPanel::sortSuggestions);
}

void SuggestionPanel::setModel(MentalModel* newModel) {
    model = newModel;
    clearSuggestions();
}

void SuggestionPanel::addSuggestion(const LinkSuggestion& suggestion) {
    suggestions.append(suggestion);
    
    auto item = new QTreeWidgetItem(suggestionsTree);
    
    // Get concept names instead of IDs (Bug fix #1)
    std::string sourceName = suggestion.sourceConceptId;
    std::string targetName = suggestion.targetConceptId;
    if (model) {
        auto sourceConcept = model->getConcept(suggestion.sourceConceptId);
        auto targetConcept = model->getConcept(suggestion.targetConceptId);
        if (sourceConcept) sourceName = sourceConcept->getName();
        if (targetConcept) targetName = targetConcept->getName();
    }
    
    item->setText(0, QString::fromStdString(sourceName));
    item->setText(1, QString::fromStdString(targetName));
    item->setText(2, QString::fromStdString(suggestion.algorithmName));  // Bug fix #2: Use algorithm name instead of suggestedType
    item->setText(3, QString::number(suggestion.confidence, 'f', 3));
    
    // Set confidence-based color coding
    QColor color;
    if (suggestion.confidence >= 0.8) {
        color = QColor(76, 175, 80, 50); // Green for high confidence
    } else if (suggestion.confidence >= 0.6) {
        color = QColor(255, 193, 7, 50); // Yellow for medium confidence
    } else {
        color = QColor(244, 67, 54, 50); // Red for low confidence
    }
    
    for (int i = 0; i < 4; ++i) {
        item->setBackground(i, QBrush(color));
    }
    
    // Store suggestion data in item
    item->setData(0, Qt::UserRole, suggestions.size() - 1);
    updateSuggestionCount();
}

void SuggestionPanel::clearSuggestions() {
    suggestions.clear();
    suggestionsTree->clear();
    acceptButton->setEnabled(false);
    rejectButton->setEnabled(false);
    updateSuggestionCount();
}

void SuggestionPanel::generateSuggestions() {
    if (!model) {
        QMessageBox::warning(this, "No Model", "No model available for generating suggestions.");
        return;
    }
    
    try {
        clearSuggestions();
        
        // Show progress
        if (progressBar) {
            progressBar->setVisible(true);
            progressBar->setRange(0, 0); // Indeterminate
        }
        if (generateButton) {
            generateButton->setEnabled(false);
            generateButton->setText("Generating...");
        }
        
        // Get selected algorithm
        QString algorithm = algorithmCombo ? algorithmCombo->currentData().toString() : "common_neighbors";
        double minConfidence = confidenceThreshold ? confidenceThreshold->text().toDouble() : 0.5;
        
        // Generate suggestions using the selected algorithm
        if (algorithm == "all") {
            generateCombinedSuggestions(minConfidence);
        } else {
            generateRealSuggestions(algorithm, minConfidence);
        }
        
        // Hide progress
        if (progressBar) {
            progressBar->setVisible(false);
        }
        if (generateButton) {
            generateButton->setEnabled(true);
            generateButton->setText("Generate Suggestions");
        }
        
        emit suggestionsGenerated(suggestions.size());
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", 
            QString("Error generating suggestions: %1").arg(e.what()));
        // Reset UI state
        if (progressBar) {
            progressBar->setVisible(false);
        }
        if (generateButton) {
            generateButton->setEnabled(true);
            generateButton->setText("Generate Suggestions");
        }
    }
}



void SuggestionPanel::generateRealSuggestions(const QString& algorithm, double minConfidence) {
    if (!model) return;
    
    try {
        std::unique_ptr<ILinkPredictor> predictor;
        
        // Create the appropriate predictor based on algorithm selection
        if (algorithm == "common_neighbors") {
            predictor = std::make_unique<CommonNeighborPredictor>(this);
        } else if (algorithm == "jaccard") {
            predictor = std::make_unique<JaccardCoefficientPredictor>(this);
        } else if (algorithm == "preferential") {
            predictor = std::make_unique<PreferentialAttachmentPredictor>(this);
        } else {
            // Default to common neighbors
            predictor = std::make_unique<CommonNeighborPredictor>(this);
        }
        
        // Generate suggestions using the selected predictor
        auto predictedLinks = predictor->predictLinks(*model, 10);
        
        // Filter by confidence threshold and add to UI
        for (const auto& suggestion : predictedLinks) {
            if (suggestion.confidence >= minConfidence) {
                addSuggestion(suggestion);
            }
        }
        
    } catch (const std::exception& e) {
        qDebug() << "Error in generateRealSuggestions:" << e.what();
    }
}

void SuggestionPanel::generateCombinedSuggestions(double minConfidence) {
    if (!model) return;
    
    try {
        // Create all predictors
        auto commonNeighbor = std::make_unique<CommonNeighborPredictor>(this);
        auto jaccard = std::make_unique<JaccardCoefficientPredictor>(this);
        auto preferential = std::make_unique<PreferentialAttachmentPredictor>(this);
        
        // Get suggestions from each algorithm
        auto cnSuggestions = commonNeighbor->predictLinks(*model, 10);
        auto jcSuggestions = jaccard->predictLinks(*model, 10);
        auto paSuggestions = preferential->predictLinks(*model, 10);
        
        // Combine and deduplicate suggestions
        std::map<std::pair<std::string, std::string>, std::vector<LinkSuggestion>> combinedMap;
        
        for (const auto& suggestion : cnSuggestions) {
            auto key = std::make_pair(suggestion.sourceConceptId, suggestion.targetConceptId);
            combinedMap[key].push_back(suggestion);
        }
        
        for (const auto& suggestion : jcSuggestions) {
            auto key = std::make_pair(suggestion.sourceConceptId, suggestion.targetConceptId);
            combinedMap[key].push_back(suggestion);
        }
        
        for (const auto& suggestion : paSuggestions) {
            auto key = std::make_pair(suggestion.sourceConceptId, suggestion.targetConceptId);
            combinedMap[key].push_back(suggestion);
        }
        
        // Create combined suggestions with averaged confidence
        for (const auto& pair : combinedMap) {
            const auto& suggestions = pair.second;
            if (suggestions.empty()) continue;
            
            double avgConfidence = 0.0;
            std::string combinedExplanation = "Combined prediction from multiple algorithms:\n";
            
            for (const auto& suggestion : suggestions) {
                avgConfidence += suggestion.confidence;
                combinedExplanation += "- " + suggestion.explanation + "\n";
            }
            
            avgConfidence /= suggestions.size();
            
            if (avgConfidence >= minConfidence) {
                LinkSuggestion combined(
                    suggestions[0].sourceConceptId,
                    suggestions[0].targetConceptId,
                    "predicted_relationship",
                    avgConfidence,
                    combinedExplanation,
                    "Combined Algorithms"  // Algorithm name for combined suggestions
                );
                
                addSuggestion(combined);
            }
        }
        
    } catch (const std::exception& e) {
        qDebug() << "Error in generateCombinedSuggestions:" << e.what();
    }
}

void SuggestionPanel::acceptSuggestion() {
    auto currentItem = suggestionsTree->currentItem();
    if (!currentItem || !model) return;
    
    int index = currentItem->data(0, Qt::UserRole).toInt();
    if (index < 0 || index >= suggestions.size()) return;
    
    const auto& suggestion = suggestions[index];
    
    // Create and add the relationship to the model
    auto relationship = std::make_unique<Relationship>(
        suggestion.sourceConceptId,
        suggestion.targetConceptId,
        "related_to", // default type
        true, // directed
        suggestion.confidence
    );
    
    model->addRelationship(std::move(relationship));
    
    // Remove from suggestions
    suggestions.removeAt(index);
    delete currentItem;
    
    // Update UI
    updateSuggestionCount();
    acceptButton->setEnabled(false);
    rejectButton->setEnabled(false);
    
    emit suggestionAccepted(suggestion);
}

void SuggestionPanel::rejectSuggestion() {
    auto currentItem = suggestionsTree->currentItem();
    if (!currentItem) return;

    int index = currentItem->data(0, Qt::UserRole).toInt();
    if (index < 0 || index >= suggestions.size()) return;

    const auto& suggestion = suggestions[index];
    
    // Remove from suggestions
    suggestions.removeAt(index);
    delete currentItem;
    
    // Update UI
    updateSuggestionCount();
    acceptButton->setEnabled(false);
    rejectButton->setEnabled(false);
    
    emit suggestionRejected(suggestion);
}

void SuggestionPanel::onSelectionChanged() {
    auto currentItem = suggestionsTree->currentItem();
    if (!currentItem) {
        acceptButton->setEnabled(false);
        rejectButton->setEnabled(false);
        return;
    }

    int index = currentItem->data(0, Qt::UserRole).toInt();
    if (index < 0 || index >= suggestions.size()) return;

    const auto& suggestion = suggestions[index];
    
    // Get concept names for details display
    std::string sourceName = suggestion.sourceConceptId;
    std::string targetName = suggestion.targetConceptId;
    if (model) {
        auto sourceConcept = model->getConcept(suggestion.sourceConceptId);
        auto targetConcept = model->getConcept(suggestion.targetConceptId);
        if (sourceConcept) sourceName = sourceConcept->getName();
        if (targetConcept) targetName = targetConcept->getName();
    }
    
    // Show modal dialog with details
    QString details = QString(
        "<h3>Link Suggestion Details</h3>"
        "<table style='width:100%; margin-top:10px;'>"
        "<tr><td><b>Source Concept:</b></td><td>%1</td></tr>"
        "<tr><td><b>Target Concept:</b></td><td>%2</td></tr>"
        "<tr><td><b>Algorithm Used:</b></td><td>%3</td></tr>"
        "<tr><td><b>Confidence:</b></td><td>%4</td></tr>"
        "</table>"
        "<h4 style='margin-top:15px;'>Explanation:</h4>"
        "<p style='margin-left:10px;'>%5</p>"
    ).arg(QString::fromStdString(sourceName))
     .arg(QString::fromStdString(targetName))
     .arg(QString::fromStdString(suggestion.algorithmName))
     .arg(suggestion.confidence, 0, 'f', 3)
     .arg(QString::fromStdString(suggestion.explanation).replace("\n", "<br>"));
    
    QMessageBox detailsDialog(this);
    detailsDialog.setWindowTitle("AI Suggestion Details");
    detailsDialog.setTextFormat(Qt::RichText);
    detailsDialog.setText(details);
    detailsDialog.setIcon(QMessageBox::Information);
    detailsDialog.setStandardButtons(QMessageBox::Ok);
    detailsDialog.exec();
    acceptButton->setEnabled(true);
    rejectButton->setEnabled(true);
}

void SuggestionPanel::filterSuggestions() {
    QString filterText = filterEdit->text().toLower();
    
    for (int i = 0; i < suggestionsTree->topLevelItemCount(); ++i) {
        auto item = suggestionsTree->topLevelItem(i);
        bool visible = filterText.isEmpty() ||
                      item->text(0).toLower().contains(filterText) ||
                      item->text(1).toLower().contains(filterText) ||
                      item->text(2).toLower().contains(filterText);
        item->setHidden(!visible);
    }
}

void SuggestionPanel::sortSuggestions() {
    QString sortBy = sortCombo->currentData().toString();
    int column = 0;
    
    if (sortBy == "confidence") column = 3;
    else if (sortBy == "source") column = 0;
    else if (sortBy == "target") column = 1;
    else if (sortBy == "algorithm") column = 2;
    
    suggestionsTree->sortItems(column, Qt::DescendingOrder);
}

void SuggestionPanel::updateSuggestionCount() {
    int totalCount = suggestions.size();
    int visibleCount = 0;
    
    for (int i = 0; i < suggestionsTree->topLevelItemCount(); ++i) {
        if (!suggestionsTree->topLevelItem(i)->isHidden()) {
            visibleCount++;
        }
    }
    
    QString countText = QString("Suggestions (%1/%2)").arg(visibleCount).arg(totalCount);
    // Update the group box title if needed
    auto parent = qobject_cast<QGroupBox*>(suggestionsTree->parent()->parent());
    if (parent) {
        parent->setTitle(countText);
    }
}

} // namespace qlink