#pragma once

#include <QWidget>
#include <QList>
#include "../core/model/MentalModel.h"
#include "../core/common/DataStructures.h"

class QVBoxLayout;
class QHBoxLayout;
class QComboBox;
class QLineEdit;
class QPushButton;
class QProgressBar;
class QTreeWidget;
class QTreeWidgetItem;
class QTextEdit;

namespace qlink {

/**
 * Panel for displaying and managing AI generated link suggestions
 */
class SuggestionPanel : public QWidget {
    Q_OBJECT

public:
    explicit SuggestionPanel(QWidget *parent = nullptr);
    ~SuggestionPanel();

    void setModel(MentalModel* model);
    void addSuggestion(const LinkSuggestion& suggestion);
    void clearSuggestions();

signals:
    void suggestionsGenerated(int count);
    void suggestionAccepted(const LinkSuggestion& suggestion);
    void suggestionRejected(const LinkSuggestion& suggestion);

public slots:
    void generateSuggestions();

private slots:
    void acceptSuggestion();
    void rejectSuggestion();
    void onSelectionChanged();
    void filterSuggestions();
    void sortSuggestions();

private:
    void setupUI();
    void setupControlsSection(QVBoxLayout* mainLayout);
    void setupSuggestionsSection(QVBoxLayout* mainLayout);
    void setupActionButtons(QVBoxLayout* mainLayout);
    void setupConnections();
    void generateRealSuggestions(const QString& algorithm, double minConfidence);
    void generateCombinedSuggestions(double minConfidence);
    void updateSuggestionCount();

    // Core components
    MentalModel* model;
    QList<LinkSuggestion> suggestions;

    // UI components
    QComboBox* algorithmCombo;
    QLineEdit* confidenceThreshold;
    QPushButton* generateButton;
    QProgressBar* progressBar;
    
    QLineEdit* filterEdit;
    QComboBox* sortCombo;
    QTreeWidget* suggestionsTree;
    
    QPushButton* acceptButton;
    QPushButton* rejectButton;
    QPushButton* clearButton;
};

} // namespace qlink