#pragma once
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QMap>
#include <QTimer>
#include <QContextMenuEvent>
#include "../core/model/MentalModel.h"
#include "../core/model/Concept.h"
#include "../core/model/Relationship.h"

namespace qlink {

class ConceptGraphicsItem;
class RelationshipGraphicsItem;

/**
 * Widget for displaying and interacting with the mental model graph
 */
class GraphWidget : public QGraphicsView {
    Q_OBJECT

public:
    explicit GraphWidget(QWidget *parent = nullptr);
    ~GraphWidget();

    void setModel(MentalModel* model);

public slots:
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void fitToWindow();

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private slots:
    void onConceptAdded(const QString& conceptId);
    void onConceptRemoved(const QString& conceptId);
    void onRelationshipAdded(const QString& relationshipId);
    void onRelationshipRemoved(const QString& relationshipId);

private:
    void setupView();
    void setupScene();
    void rebuildGraph();
    void createConceptItem(const Concept* concept);
    void createRelationshipItem(const Relationship* relationship);
    void initializePositions();
    void scaleView(double scaleFactor);
    
    // AI Assistant methods
    void showConceptAIExplanation(ConceptGraphicsItem* conceptItem);
    void generateConceptDescription(ConceptGraphicsItem* conceptItem);
    void suggestRelatedConcepts(ConceptGraphicsItem* conceptItem);
    void showRelationshipAIExplanation(RelationshipGraphicsItem* relationshipItem);

    // Core components
    QGraphicsScene* scene;
    MentalModel* model;

    // Visual items
    QMap<std::string, ConceptGraphicsItem*> conceptItems;
    QMap<std::string, RelationshipGraphicsItem*> relationshipItems;

    // Interaction state
    double zoomFactor;
    ConceptGraphicsItem* selectedConcept;
    bool isDragging;
    QPoint dragStartPos;
    
    // Layout stability tracking
    double totalMovement;
    int stableIterations;
    const double STABILITY_THRESHOLD = 0.5;  // Threshold for considering layout stable
    const int STABLE_COUNT_REQUIRED = 10;    // Number of stable iterations before slowing down
};

/**
 * Graphics item representing a concept node
 */
class ConceptGraphicsItem : public QGraphicsEllipseItem {
public:
    explicit ConceptGraphicsItem(const Concept* concept, QGraphicsItem* parent = nullptr);
    const Concept* getConcept() const { return concept; }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    const Concept* concept;
    QGraphicsTextItem* textItem;
};

/**
 * Graphics item representing a relationship edge
 */
class RelationshipGraphicsItem : public QGraphicsLineItem {
public:
    explicit RelationshipGraphicsItem(const Relationship* relationship,
                                    ConceptGraphicsItem* source,
                                    ConceptGraphicsItem* target,
                                    QGraphicsItem* parent = nullptr);
    const Relationship* getRelationship() const { return relationship; }
    void updatePosition();

private:
    const Relationship* relationship;
    ConceptGraphicsItem* sourceItem;
    ConceptGraphicsItem* targetItem;
};

} // namespace qlink