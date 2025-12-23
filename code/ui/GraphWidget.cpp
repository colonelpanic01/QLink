#include "GraphWidget.h"
#include "../core/ai/AIAssistant.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QContextMenuEvent>
#include <QApplication>
#include <QScrollBar>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QGraphicsProxyWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QRandomGenerator>
#include <QDebug>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <cmath>

namespace qlink {

GraphWidget::GraphWidget(QWidget *parent)
    : QGraphicsView(parent), model(nullptr), zoomFactor(1.0), 
      selectedConcept(nullptr), isDragging(false),
      totalMovement(0.0), stableIterations(0) {
    setupView();
    setupScene();
    startLayoutTimer();
}

GraphWidget::~GraphWidget() = default;

void GraphWidget::setupView() {
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::RubberBandDrag);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    
    // Enable mouse tracking for hover effects
    setMouseTracking(true);
    
    // Set background
    setBackgroundBrush(QBrush(QColor(240, 240, 242)));
}

void GraphWidget::setupScene() {
    scene = new QGraphicsScene(this);
    scene->setSceneRect(-1000, -1000, 2000, 2000);
    setScene(scene);
}

void GraphWidget::startLayoutTimer() {
    layoutTimer = new QTimer(this);
    connect(layoutTimer, &QTimer::timeout, this, &GraphWidget::updateLayout);
    layoutTimer->start(50); // Update layout 20 times per second
}

void GraphWidget::setModel(MentalModel* newModel) {
    // Only disconnect if we have a model and it's changing
    if (model && model != newModel) {
        // Safely disconnect - the model should still be valid at this point
        disconnect(model, nullptr, this, nullptr);
    }
    
    model = newModel;
    
    if (model) {
        // Connect to new model signals
        connect(model, &MentalModel::conceptAdded, this, &GraphWidget::onConceptAdded);
        connect(model, &MentalModel::conceptRemoved, this, &GraphWidget::onConceptRemoved);
        connect(model, &MentalModel::relationshipAdded, this, &GraphWidget::onRelationshipAdded);
        connect(model, &MentalModel::relationshipRemoved, this, &GraphWidget::onRelationshipRemoved);
        
        // Rebuild the entire graph
        rebuildGraph();
    }
}

void GraphWidget::rebuildGraph() {
    if (!model) return;
    
    // Clear existing items
    scene->clear();
    conceptItems.clear();
    relationshipItems.clear();
    
    // Add all concepts
    for (const auto& concept : model->getConcepts()) {
        createConceptItem(concept.get());
    }
    
    // Add all relationships
    for (const auto& relationship : model->getRelationships()) {
        createRelationshipItem(relationship.get());
    }
    
    // Initialize positions if needed
    initializePositions();
}

void GraphWidget::createConceptItem(const Concept* concept) {
    if (!concept) return;
    
    auto item = new ConceptGraphicsItem(concept);
    scene->addItem(item);
    conceptItems[concept->getId()] = item;
    
    // Set initial position
    auto pos = concept->getPosition();
    if (pos.x == 0.0 && pos.y == 0.0) {
        // Generate random position if not set
        double x = QRandomGenerator::global()->bounded(-400, 400);
        double y = QRandomGenerator::global()->bounded(-300, 300);
        item->setPos(x, y);
    } else {
        item->setPos(pos.x, pos.y);
    }
}

void GraphWidget::createRelationshipItem(const Relationship* relationship) {
    if (!relationship) return;
    
    auto sourceItem = conceptItems.value(relationship->getSourceConceptId());
    auto targetItem = conceptItems.value(relationship->getTargetConceptId());
    
    if (sourceItem && targetItem) {
        auto item = new RelationshipGraphicsItem(relationship, sourceItem, targetItem);
        scene->addItem(item);
        relationshipItems[relationship->getId()] = item;
    }
}

void GraphWidget::initializePositions() {
    if (conceptItems.isEmpty()) return;
    
    // Check if concepts already have positions (from loaded file)
    bool hasExistingPositions = false;
    for (auto it = conceptItems.begin(); it != conceptItems.end(); ++it) {
        auto concept = model->getConcept(it.key());
        if (concept) {
            auto pos = concept->getPosition();
            if (pos.x != 0.0 || pos.y != 0.0) {
                hasExistingPositions = true;
                it.value()->setPos(pos.x, pos.y);
            }
        }
    }
    
    // If no existing positions, create a better spread layout
    if (!hasExistingPositions) {
        int count = conceptItems.size();
        double radius = std::max(300.0, count * 50.0); // Larger radius for better spacing
        double angleStep = 2.0 * M_PI / count;
        
        int index = 0;
        for (auto it = conceptItems.begin(); it != conceptItems.end(); ++it, ++index) {
            double angle = index * angleStep;
            double x = radius * std::cos(angle);
            double y = radius * std::sin(angle);
            
            // Add some randomness to prevent perfect overlap
            x += (QRandomGenerator::global()->bounded(-50, 50));
            y += (QRandomGenerator::global()->bounded(-50, 50));
            
            it.value()->setPos(x, y);
        }
    }
}

void GraphWidget::updateLayout() {
    if (!model || conceptItems.isEmpty()) return;
    
    // Improved force-directed layout algorithm with better spacing
    const double repulsionStrength = 20000.0; // Stronger repulsion to prevent bunching
    const double attractionStrength = 0.05;   // Weaker attraction to allow more spread
    const double damping = 0.95;              // Higher damping for stability
    const double minDistance = 150.0;         // Larger minimum distance between nodes
    
    // Calculate forces for each concept
    QMap<QString, QPointF> forces;
    
    // Initialize forces
    for (auto it = conceptItems.begin(); it != conceptItems.end(); ++it) {
        forces[QString::fromStdString(it.key())] = QPointF(0, 0);
    }
    
    // Repulsion forces (concepts push each other away)
    for (auto it1 = conceptItems.begin(); it1 != conceptItems.end(); ++it1) {
        for (auto it2 = std::next(it1); it2 != conceptItems.end(); ++it2) {
            QPointF pos1 = it1.value()->pos();
            QPointF pos2 = it2.value()->pos();
            QPointF diff = pos1 - pos2;
            double distance = std::sqrt(diff.x() * diff.x() + diff.y() * diff.y());
            
            if (distance < minDistance) distance = minDistance;
            
            double force = repulsionStrength / (distance * distance);
            QPointF forceVector = diff * (force / distance);
            
            forces[QString::fromStdString(it1.key())] += forceVector;
            forces[QString::fromStdString(it2.key())] -= forceVector;
        }
    }
    
    // Attraction forces (connected concepts pull each other) - with limits to prevent bunching
    const double idealConnectionDistance = 200.0; // Ideal distance between connected nodes
    const double maxAttractionDistance = 400.0;   // Don't attract if too far apart
    
    for (const auto& relationship : model->getRelationships()) {
        auto sourceItem = conceptItems.value(relationship->getSourceConceptId());
        auto targetItem = conceptItems.value(relationship->getTargetConceptId());
        
        if (sourceItem && targetItem) {
            QPointF pos1 = sourceItem->pos();
            QPointF pos2 = targetItem->pos();
            QPointF diff = pos2 - pos1;
            double distance = std::sqrt(diff.x() * diff.x() + diff.y() * diff.y());
            
            if (distance > 0 && distance < maxAttractionDistance) {
                // Use spring-like force that's strongest when distance != ideal
                double distanceError = distance - idealConnectionDistance;
                double force = attractionStrength * distanceError * relationship->getWeight();
                
                // Cap the force to prevent too strong attraction
                force = std::max(-2.0, std::min(2.0, force));
                
                QPointF forceVector = diff * (force / distance);
                
                forces[QString::fromStdString(relationship->getSourceConceptId())] += forceVector;
                forces[QString::fromStdString(relationship->getTargetConceptId())] -= forceVector;
            }
        }
    }
    
    // Track total movement to detect stability
    totalMovement = 0.0;
    
    // Apply forces to move the concepts
    for (auto it = conceptItems.begin(); it != conceptItems.end(); ++it) {
        if (it.value()->isSelected()) continue; // Don't move selected items
        
        QPointF currentPos = it.value()->pos();
        QPointF force = forces[QString::fromStdString(it.key())];
        QPointF newPos = currentPos + force * damping;
        
        // Keep within reasonable bounds
        newPos.setX(std::max(-800.0, std::min(800.0, newPos.x())));
        newPos.setY(std::max(-600.0, std::min(600.0, newPos.y())));
        
        // Calculate movement distance
        QPointF movement = newPos - currentPos;
        totalMovement += std::sqrt(movement.x() * movement.x() + movement.y() * movement.y());
        
        it.value()->setPos(newPos);
    }
    
    // Check if layout is stable
    if (totalMovement < STABILITY_THRESHOLD) {
        stableIterations++;
        
        // If stable for long enough, slow down or pause updates
        if (stableIterations >= STABLE_COUNT_REQUIRED) {
            // Slow down timer to reduce CPU usage when layout is stable
            if (layoutTimer->interval() == 50) {
                layoutTimer->setInterval(500); // Update only twice per second when stable
            }
        }
    } else {
        stableIterations = 0;
        // If movement detected, resume fast updates
        if (layoutTimer->interval() != 50) {
            layoutTimer->setInterval(50);
        }
    }
    
    // Update relationship items
    for (auto item : relationshipItems) {
        item->updatePosition();
    }
}

// Zoom and view operations
void GraphWidget::zoomIn() {
    scaleView(1.25);
}

void GraphWidget::zoomOut() {
    scaleView(0.8);
}

void GraphWidget::resetZoom() {
    resetTransform();
    zoomFactor = 1.0;
}

void GraphWidget::fitToWindow() {
    if (scene->items().isEmpty()) return;
    
    QRectF itemsRect = scene->itemsBoundingRect();
    fitInView(itemsRect, Qt::KeepAspectRatio);
    zoomFactor = transform().m11(); // Update zoom factor
}

void GraphWidget::scaleView(double scaleFactor) {
    double factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.1 || factor > 10.0) return; // Limit zoom range
    
    scale(scaleFactor, scaleFactor);
    zoomFactor *= scaleFactor;
}

// Event handlers
void GraphWidget::wheelEvent(QWheelEvent* event) {
    if (event->modifiers() & Qt::ControlModifier) {
        // Zoom with Ctrl+Wheel
        const double scaleFactor = 1.15;
        if (event->angleDelta().y() > 0) {
            scaleView(scaleFactor);
        } else {
            scaleView(1.0 / scaleFactor);
        }
        event->accept();
    } else {
        QGraphicsView::wheelEvent(event);
    }
}

void GraphWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        QGraphicsItem* item = itemAt(event->pos());
        if (auto conceptItem = dynamic_cast<ConceptGraphicsItem*>(item)) {
            selectedConcept = conceptItem;
            isDragging = true;
            dragStartPos = event->pos();
        }
    }
    QGraphicsView::mousePressEvent(event);
}

void GraphWidget::mouseMoveEvent(QMouseEvent* event) {
    if (isDragging && selectedConcept) {
        // Update concept position
        QPointF scenePos = mapToScene(event->pos());
        selectedConcept->setPos(scenePos);
        
        // Update relationships
        for (auto item : relationshipItems) {
            item->updatePosition();
        }
    }
    QGraphicsView::mouseMoveEvent(event);
}

void GraphWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        isDragging = false;
        selectedConcept = nullptr;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void GraphWidget::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
    case Qt::Key_Plus:
    case Qt::Key_Equal:
        if (event->modifiers() & Qt::ControlModifier) {
            zoomIn();
            return;
        }
        break;
    case Qt::Key_Minus:
        if (event->modifiers() & Qt::ControlModifier) {
            zoomOut();
            return;
        }
        break;
    case Qt::Key_0:
        if (event->modifiers() & Qt::ControlModifier) {
            resetZoom();
            return;
        }
        break;
    }
    QGraphicsView::keyPressEvent(event);
}

// Model change handlers
void GraphWidget::onConceptAdded(const QString& conceptId) {
    if (!model) return;
    auto concept = model->getConcept(conceptId.toStdString());
    if (concept) {
        createConceptItem(concept);
        // Reset stability tracking to restart layout animation
        stableIterations = 0;
        if (layoutTimer->interval() != 50) {
            layoutTimer->setInterval(50);
        }
    }
}

void GraphWidget::onConceptRemoved(const QString& conceptId) {
    auto item = conceptItems.take(conceptId.toStdString());
    if (item) {
        scene->removeItem(item);
        delete item;
        // Reset stability tracking
        stableIterations = 0;
        if (layoutTimer->interval() != 50) {
            layoutTimer->setInterval(50);
        }
    }
}

void GraphWidget::onRelationshipAdded(const QString& relationshipId) {
    if (!model) return;
    auto relationship = model->getRelationship(relationshipId.toStdString());
    if (relationship) {
        createRelationshipItem(relationship);
        // Reset stability tracking
        stableIterations = 0;
        if (layoutTimer->interval() != 50) {
            layoutTimer->setInterval(50);
        }
    }
}

void GraphWidget::onRelationshipRemoved(const QString& relationshipId) {
    auto item = relationshipItems.take(relationshipId.toStdString());
    if (item) {
        scene->removeItem(item);
        delete item;
        // Reset stability tracking
        stableIterations = 0;
        if (layoutTimer->interval() != 50) {
            layoutTimer->setInterval(50);
        }
    }
}

// ConceptGraphicsItem implementation
ConceptGraphicsItem::ConceptGraphicsItem(const Concept* concept, QGraphicsItem* parent)
    : QGraphicsEllipseItem(parent), concept(concept) {
    setRect(-35, -35, 70, 70);
    
    setBrush(QBrush(QColor(220, 240, 255)));
    setPen(QPen(QColor(50, 100, 200), 3));
    
    // Make it selectable and movable
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    
    textItem = new QGraphicsTextItem(QString::fromStdString(concept->getName()), this);
    textItem->setDefaultTextColor(QColor(0, 0, 0));
    textItem->setFont(QFont("Arial", 9, QFont::Bold));
    
    QRectF textRect = textItem->boundingRect();
    textItem->setPos(-textRect.width() / 2, 40);
    
    // Set tooltip
    setToolTip(QString::fromStdString(concept->getDescription()));
}

QVariant ConceptGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == ItemPositionHasChanged) {
        QPointF newPos = value.toPointF();
        // update the model here
        // const_cast<Concept*>(concept)->setPosition({newPos.x(), newPos.y()});
    }
    return QGraphicsEllipseItem::itemChange(change, value);
}

// RelationshipGraphicsItem implementation
RelationshipGraphicsItem::RelationshipGraphicsItem(const Relationship* relationship,
                                                 ConceptGraphicsItem* source,
                                                 ConceptGraphicsItem* target,
                                                 QGraphicsItem* parent)
    : QGraphicsLineItem(parent), relationship(relationship), sourceItem(source), targetItem(target) {
    // Set up the visual appearance
    double weight = relationship->getWeight();
    int penWidth = static_cast<int>(1 + weight * 3); // 1-4 pixels based on weight
    
    QColor color;
    if (weight >= 0.8) {
        color = QColor(46, 204, 113); // Strong - green
    } else if (weight >= 0.6) {
        color = QColor(241, 196, 15); // Medium - yellow
    } else if (weight >= 0.4) {
        color = QColor(230, 126, 34); // Weak - orange
    } else {
        color = QColor(231, 76, 60); // Very weak - red
    }
    
    setPen(QPen(color, penWidth));
    
    // Set tooltip
    QString strengthStr = weight >= 0.8 ? "Strong" : 
                         weight >= 0.6 ? "Medium" : 
                         weight >= 0.4 ? "Weak" : "Very Weak";
    QString tooltip = QString("Type: %1\nWeight: %2\nStrength: %3")
                     .arg(QString::fromStdString(relationship->getType()))
                     .arg(weight, 0, 'f', 2)
                     .arg(strengthStr);
    setToolTip(tooltip);
    
    updatePosition();
}

void RelationshipGraphicsItem::updatePosition() {
    if (sourceItem && targetItem) {
        QPointF sourcePos = sourceItem->pos();
        QPointF targetPos = targetItem->pos();
        setLine(QLineF(sourcePos, targetPos));
    }
}

void GraphWidget::contextMenuEvent(QContextMenuEvent* event) {
    QGraphicsItem* item = itemAt(event->pos());
    
    if (!item) {
        // Right-clicked on empty space
        QMenu menu(this);
        menu.addAction("Add Concept", [this, event]() {
            // TODO: Add concept creation dialog
            QPointF scenePos = mapToScene(event->pos());
            qDebug() << "Add concept at:" << scenePos;
        });
        menu.exec(event->globalPos());
        return;
    }
    
    // Check if it's a concept item
    ConceptGraphicsItem* conceptItem = dynamic_cast<ConceptGraphicsItem*>(item);
    if (conceptItem) {
        QMenu menu(this);
        
        menu.addAction("AI Explanation", [this, conceptItem]() {
            showConceptAIExplanation(conceptItem);
        });
        
        menu.addAction("Generate AI Description", [this, conceptItem]() {
            generateConceptDescription(conceptItem);
        });
        
        menu.addAction("Suggest Related Concepts", [this, conceptItem]() {
            suggestRelatedConcepts(conceptItem);
        });
        
        menu.addSeparator();
        menu.addAction("Delete Concept", [this, conceptItem]() {
            if (!model) return;
            
            const Concept* concept = conceptItem->getConcept();
            std::string conceptId = concept->getId();
            std::string conceptName = concept->getName();
            
            QMessageBox::StandardButton reply = QMessageBox::question(
                this,
                "Delete Concept",
                QString("Are you sure you want to delete '%1'?").arg(QString::fromStdString(conceptName)),
                QMessageBox::Yes | QMessageBox::No
            );
            
            if (reply == QMessageBox::Yes) {
                model->removeConcept(conceptId);
            }
        });
        
        menu.exec(event->globalPos());
        return;
    }
    
    // Check if it's a relationship item
    RelationshipGraphicsItem* relationshipItem = dynamic_cast<RelationshipGraphicsItem*>(item);
    if (relationshipItem) {
        QMenu menu(this);
        
        menu.addAction("AI Explanation", [this, relationshipItem]() {
            showRelationshipAIExplanation(relationshipItem);
        });
        
        menu.addSeparator();
        menu.addAction("Delete Relationship", [this, relationshipItem]() {
            if (!model) return;
            
            const Relationship* relationship = relationshipItem->getRelationship();
            QString relType = QString::fromStdString(relationship->getType());
            
            QMessageBox::StandardButton reply = QMessageBox::question(
                this,
                "Delete Relationship",
                QString("Are you sure you want to delete this '%1' relationship?").arg(relType),
                QMessageBox::Yes | QMessageBox::No
            );
            
            if (reply == QMessageBox::Yes) {
                qDebug() << "Delete relationship:" << relationship->getType().c_str();
                model->removeRelationship(relationship->getId());
            }
        });
        
        menu.exec(event->globalPos());
        return;
    }
}

void GraphWidget::showConceptAIExplanation(ConceptGraphicsItem* conceptItem) {
    if (!conceptItem || !model) return;
    
    AIAssistant assistant;
    if (!assistant.isServiceAvailable()) {
        QMessageBox::information(this, "AI Assistant", 
            "AI service is not available. Please set the COHERE_API_KEY environment variable.");
        return;
    }
    
    const Concept* concept = conceptItem->getConcept();
    std::string description = assistant.generateConceptDescription(concept->getName());
    
    QMessageBox::information(this, "AI Concept Explanation", 
        QString("Concept: %1\n\n%2")
        .arg(QString::fromStdString(concept->getName()))
        .arg(QString::fromStdString(description)));
}

void GraphWidget::generateConceptDescription(ConceptGraphicsItem* conceptItem) {
    if (!conceptItem || !model) return;
    
    AIAssistant assistant;
    if (!assistant.isServiceAvailable()) {
        QMessageBox::information(this, "AI Assistant", 
            "AI service is not available. Please set the COHERE_API_KEY environment variable.");
        return;
    }
    
    const Concept* concept = conceptItem->getConcept();
    std::string description = assistant.generateConceptDescription(concept->getName());
    
    // TODO: Actually update the concept's description in the model
    QMessageBox::information(this, "Generated Description", 
        QString("Generated description for '%1':\n\n%2")
        .arg(QString::fromStdString(concept->getName()))
        .arg(QString::fromStdString(description)));
}

void GraphWidget::suggestRelatedConcepts(ConceptGraphicsItem* conceptItem) {
    if (!conceptItem || !model) return;
    
    AIAssistant assistant;
    if (!assistant.isServiceAvailable()) {
        QMessageBox::information(this, "AI Assistant", 
            "AI service is not available. Please set the COHERE_API_KEY environment variable.");
        return;
    }
    
    const Concept* concept = conceptItem->getConcept();
    std::vector<std::string> suggestions = assistant.suggestRelatedConcepts(*concept);
    
    QString suggestionsText = "Suggested related concepts:\n\n";
    for (const auto& suggestion : suggestions) {
        suggestionsText += "• " + QString::fromStdString(suggestion) + "\n";
    }
    
    QMessageBox::information(this, "Concept Suggestions", suggestionsText);
}

void GraphWidget::showRelationshipAIExplanation(RelationshipGraphicsItem* relationshipItem) {
    if (!relationshipItem || !model) return;
    
    AIAssistant assistant;
    if (!assistant.isServiceAvailable()) {
        QMessageBox::information(this, "AI Assistant", 
            "AI service is not available. Please set the COHERE_API_KEY environment variable.");
        return;
    }
    
    const Relationship* relationship = relationshipItem->getRelationship();
    const Concept* sourceConcept = model->getConcept(relationship->getSourceConceptId());
    const Concept* targetConcept = model->getConcept(relationship->getTargetConceptId());
    
    if (sourceConcept && targetConcept) {
        std::string explanation = assistant.explainConnection(*sourceConcept, *targetConcept);
        
        QMessageBox::information(this, "AI Relationship Explanation", 
            QString("Connection: %1 → %2\nType: %3\n\n%4")
            .arg(QString::fromStdString(sourceConcept->getName()))
            .arg(QString::fromStdString(targetConcept->getName()))
            .arg(QString::fromStdString(relationship->getType()))
            .arg(QString::fromStdString(explanation)));
    }
}

} // namespace qlink