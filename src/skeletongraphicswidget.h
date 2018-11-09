#ifndef DUST3D_SKELETON_GRAPHICS_VIEW_H
#define DUST3D_SKELETON_GRAPHICS_VIEW_H
#include <map>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsPolygonItem>
#include <QThread>
#include <cmath>
#include <set>
#include <QTimer>
#include "skeletondocument.h"
#include "turnaroundloader.h"
#include "theme.h"
#include "util.h"
#include "skeletonikmover.h"
#include "modelwidget.h"

class SkeletonGraphicsOriginItem : public QGraphicsPolygonItem
{
public:
    SkeletonGraphicsOriginItem(SkeletonProfile profile=SkeletonProfile::Unknown) :
        m_profile(profile),
        m_hovered(false),
        m_checked(false)
    {
        setData(0, "origin");
    }
    void updateAppearance()
    {
        QColor color = Theme::white;
        
        switch (m_profile)
        {
        case SkeletonProfile::Unknown:
            break;
        case SkeletonProfile::Main:
            color = Theme::red;
            break;
        case SkeletonProfile::Side:
            color = Theme::green;
            break;
        }
        
        QColor penColor = color;
        penColor.setAlphaF(m_checked ? Theme::checkedAlpha : Theme::normalAlpha);
        QPen pen(penColor);
        pen.setWidth(0);
        setPen(pen);
        
        QColor brushColor = color;
        brushColor.setAlphaF((m_checked || m_hovered) ? Theme::checkedAlpha : Theme::normalAlpha);
        QBrush brush(brushColor);
        setBrush(brush);
    }
    void setOrigin(QPointF point)
    {
        m_origin = point;
        QPolygonF triangle;
        const int triangleRadius = 10;
        triangle.append(QPointF(point.x() - triangleRadius, point.y()));
        triangle.append(QPointF(point.x() + triangleRadius, point.y()));
        triangle.append(QPointF(point.x(), point.y() - triangleRadius));
        triangle.append(QPointF(point.x() - triangleRadius, point.y()));
        setPolygon(triangle);
        updateAppearance();
    }
    QPointF origin()
    {
        return m_origin;
    }
    SkeletonProfile profile()
    {
        return m_profile;
    }
    void setHovered(bool hovered)
    {
        m_hovered = hovered;
        updateAppearance();
    }
    void setChecked(bool checked)
    {
        m_checked = checked;
        updateAppearance();
    }
    bool checked()
    {
        return m_checked;
    }
    bool hovered()
    {
        return m_hovered;
    }
private:
    SkeletonProfile m_profile;
    bool m_hovered;
    bool m_checked;
    QPointF m_origin;
};

class SkeletonGraphicsSelectionItem : public QGraphicsRectItem
{
public:
    SkeletonGraphicsSelectionItem()
    {
        QColor penColor = Theme::white;
        QPen pen(penColor);
        pen.setWidth(0);
        pen.setStyle(Qt::DashLine);
        setPen(pen);
    }
    void updateRange(QPointF beginPos, QPointF endPos)
    {
        setRect(QRectF(beginPos, endPos).normalized());
    }
};

class SkeletonGraphicsNodeItem : public QGraphicsEllipseItem
{
public:
    SkeletonGraphicsNodeItem(SkeletonProfile profile=SkeletonProfile::Unknown) :
        m_profile(profile),
        m_hovered(false),
        m_checked(false),
        m_markColor(Qt::transparent)
    {
        setData(0, "node");
        setRadius(32);
    }
    void updateAppearance()
    {
        QColor color = Theme::white;
        
        switch (m_profile)
        {
        case SkeletonProfile::Unknown:
            break;
        case SkeletonProfile::Main:
            color = Theme::red;
            break;
        case SkeletonProfile::Side:
            color = Theme::green;
            break;
        }
        
        QColor penColor = color;
        penColor.setAlphaF((m_checked || m_hovered) ? Theme::checkedAlpha : Theme::normalAlpha);
        QPen pen(penColor);
        pen.setWidth(0);
        setPen(pen);
        
        QColor brushColor;
        Qt::BrushStyle style;
        if (m_markColor == Qt::transparent) {
            brushColor = color;
            brushColor.setAlphaF((m_checked || m_hovered) ? Theme::fillAlpha : 0);
            style = Qt::SolidPattern;
        } else {
            brushColor = m_markColor;
            brushColor.setAlphaF((m_checked || m_hovered) ? Theme::fillAlpha * 4 : Theme::fillAlpha * 1.5);
            style = Qt::Dense4Pattern;
        }
        if (m_checked)
            brushColor.setAlphaF(brushColor.alphaF() * 1.2);
        QBrush brush(brushColor);
        brush.setStyle(style);
        setBrush(brush);
    }
    void setOrigin(QPointF point)
    {
        QPointF moveBy = point - origin();
        QRectF newRect = rect();
        newRect.adjust(moveBy.x(), moveBy.y(), moveBy.x(), moveBy.y());
        setRect(newRect);
        updateAppearance();
    }
    QPointF origin()
    {
        return QPointF(rect().x() + rect().width() / 2,
            rect().y() + rect().height() / 2);
    }
    float radius()
    {
        return rect().width() / 2;
    }
    void setRadius(float radius)
    {
        if (radius < 1)
            radius = 1;
        QPointF oldOrigin = origin();
        setRect(oldOrigin.x() - radius, oldOrigin.y() - radius,
            radius * 2, radius * 2);
        updateAppearance();
    }
    void setMarkColor(QColor color)
    {
        m_markColor = color;
        updateAppearance();
    }
    SkeletonProfile profile()
    {
        return m_profile;
    }
    QUuid id()
    {
        return m_uuid;
    }
    void setId(QUuid id)
    {
        m_uuid = id;
    }
    void setHovered(bool hovered)
    {
        m_hovered = hovered;
        updateAppearance();
    }
    void setChecked(bool checked)
    {
        m_checked = checked;
        updateAppearance();
    }
    bool checked()
    {
        return m_checked;
    }
    bool hovered()
    {
        return m_hovered;
    }
private:
    QUuid m_uuid;
    SkeletonProfile m_profile;
    bool m_hovered;
    bool m_checked;
    QColor m_markColor;
};

class SkeletonGraphicsEdgeItem : public QGraphicsPolygonItem
{
public:
    SkeletonGraphicsEdgeItem() :
        m_firstItem(nullptr),
        m_secondItem(nullptr),
        m_hovered(false),
        m_checked(false),
        m_profile(SkeletonProfile::Unknown)
    {
        setData(0, "edge");
    }
    void setEndpoints(SkeletonGraphicsNodeItem *first, SkeletonGraphicsNodeItem *second)
    {
        m_firstItem = first;
        m_secondItem = second;
        updateAppearance();
    }
    SkeletonGraphicsNodeItem *firstItem()
    {
        return m_firstItem;
    }
    SkeletonGraphicsNodeItem *secondItem()
    {
        return m_secondItem;
    }
    void updateAppearance()
    {
        if (nullptr == m_firstItem || nullptr == m_secondItem)
            return;
        
        m_profile = m_firstItem->profile();
        
        QLineF line(m_firstItem->origin(), m_secondItem->origin());
        
        QPolygonF polygon;
        float radAngle = line.angle() * M_PI / 180;
        float dx = 2 * sin(radAngle);
        float dy = 2 * cos(radAngle);
        QPointF offset1 = QPointF(dx, dy);
        QPointF offset2 = QPointF(-dx, -dy);
        polygon << line.p1() + offset1 << line.p1() + offset2 << line.p2() + offset2 << line.p2() + offset1;
        setPolygon(polygon);
        
        QColor color = Theme::white;
        
        switch (m_firstItem->profile())
        {
        case SkeletonProfile::Unknown:
            break;
        case SkeletonProfile::Main:
            color = Theme::red;
            break;
        case SkeletonProfile::Side:
            color = Theme::green;
            break;
        }
        
        QColor penColor = color;
        penColor.setAlphaF((m_checked || m_hovered) ? Theme::checkedAlpha : Theme::normalAlpha);
        QPen pen(penColor);
        pen.setWidth(0);
        setPen(pen);
    }
    QUuid id()
    {
        return m_uuid;
    }
    void setId(QUuid id)
    {
        m_uuid = id;
    }
    SkeletonProfile profile()
    {
        return m_profile;
    }
    void setHovered(bool hovered)
    {
        m_hovered = hovered;
        updateAppearance();
    }
    void setChecked(bool checked)
    {
        m_checked = checked;
        updateAppearance();
    }
    bool checked()
    {
        return m_checked;
    }
    bool hovered()
    {
        return m_hovered;
    }
private:
    QUuid m_uuid;
    SkeletonGraphicsNodeItem *m_firstItem;
    SkeletonGraphicsNodeItem *m_secondItem;
    QPolygonF m_selectionPolygon;
    bool m_hovered;
    bool m_checked;
    SkeletonProfile m_profile;
};

class SkeletonGraphicsWidget : public QGraphicsView
{
    Q_OBJECT
signals:
    void addNode(float x, float y, float z, float radius, QUuid fromNodeId);
    void scaleNodeByAddRadius(QUuid nodeId, float amount);
    void moveNodeBy(QUuid nodeId, float x, float y, float z);
    void removeNode(QUuid nodeId);
    void setEditMode(SkeletonDocumentEditMode mode);
    void removeEdge(QUuid edgeId);
    void addEdge(QUuid fromNodeId, QUuid toNodeId);
    void cursorChanged();
    void groupOperationAdded();
    void undo();
    void redo();
    void paste();
    void changeTurnaround();
    void batchChangeBegin();
    void batchChangeEnd();
    void save();
    void open();
    void exportResult();
    void breakEdge(QUuid edgeId);
    void moveOriginBy(float x, float y, float z);
    void partChecked(QUuid partId);
    void partUnchecked(QUuid partId);
    void setPartLockState(QUuid partId, bool locked);
    void setPartVisibleState(QUuid partId, bool visible);
    void setPartSubdivState(QUuid partId, bool subdived);
    void setPartDisableState(QUuid partId, bool disabled);
    void setPartXmirrorState(QUuid partId, bool mirrored);
    void setPartRoundState(QUuid partId, bool rounded);
    void setPartWrapState(QUuid partId, bool wrapped);
    void setXlockState(bool locked);
    void setYlockState(bool locked);
    void setZlockState(bool locked);
    void setNodeOrigin(QUuid nodeId, float x, float y, float z);
    void setNodeBoneMark(QUuid nodeId, BoneMark mark);
    void zoomRenderedModelBy(float delta);
    void switchNodeXZ(QUuid nodeId);
    void switchChainSide(std::set<QUuid> nodeIds);
    void enableAllPositionRelatedLocks();
    void disableAllPositionRelatedLocks();
public:
    SkeletonGraphicsWidget(const SkeletonDocument *document);
    std::map<QUuid, std::pair<SkeletonGraphicsNodeItem *, SkeletonGraphicsNodeItem *>> nodeItemMap;
    std::map<QUuid, std::pair<SkeletonGraphicsEdgeItem *, SkeletonGraphicsEdgeItem *>> edgeItemMap;
    bool mouseMove(QMouseEvent *event);
    bool wheel(QWheelEvent *event);
    bool mouseRelease(QMouseEvent *event);
    bool mousePress(QMouseEvent *event);
    bool mouseDoubleClick(QMouseEvent *event);
    bool keyPress(QKeyEvent *event);
    bool keyRelease(QKeyEvent *event);
    bool checkSkeletonItem(QGraphicsItem *item, bool checked);
    QUuid querySkeletonItemPartId(QGraphicsItem *item);
    static SkeletonProfile readSkeletonItemProfile(QGraphicsItem *item);
    void readMergedSkeletonNodeSetFromRangeSelection(std::set<SkeletonGraphicsNodeItem *> *nodeItemSet);
    void readSkeletonNodeAndEdgeIdSetFromRangeSelection(std::set<QUuid> *nodeIdSet, std::set<QUuid> *edgeIdSet=nullptr);
    bool readSkeletonNodeAndAnyEdgeOfNodeFromRangeSelection(SkeletonGraphicsNodeItem **nodeItem, SkeletonGraphicsEdgeItem **edgeItem);
    bool hasSelection();
    bool hasItems();
    bool hasMultipleSelection();
    bool hasEdgeSelection();
    bool hasNodeSelection();
    bool hasTwoDisconnectedNodesSelection();
    void setModelWidget(ModelWidget *modelWidget);
    void setNodePositionModifyOnly(bool nodePositionModifyOnly);
protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
public slots:
    void nodeAdded(QUuid nodeId);
    void edgeAdded(QUuid edgeId);
    void nodeRemoved(QUuid nodeId);
    void edgeRemoved(QUuid edgeId);
    void nodeRadiusChanged(QUuid nodeId);
    void nodeBoneMarkChanged(QUuid nodeId);
    void nodeOriginChanged(QUuid nodeId);
    void edgeChanged(QUuid edgeId);
    void turnaroundChanged();
    void canvasResized();
    void editModeChanged();
    void updateCursor();
    void partVisibleStateChanged(QUuid partId);
    void showContextMenu(const QPoint &pos);
    void deleteSelected();
    void selectAll();
    void unselectAll();
    void selectPartAll();
    void addPartToSelection(QUuid partId);
    void cut();
    void copy();
    void flipHorizontally();
    void flipVertically();
    void rotateClockwise90Degree();
    void rotateCounterclockwise90Degree();
    void rotateAllMainProfileClockwise90DegreeAlongOrigin();
    void rotateAllMainProfileCounterclockwise90DegreeAlongOrigin();
    void removeAllContent();
    void breakSelected();
    void connectSelected();
    void rotateSelected(int degree);
    void zoomSelected(float delta);
    void scaleSelected(float delta);
    void moveSelected(float byX, float byY);
    void moveCheckedOrigin(float byX, float byY);
    void originChanged();
    void alignSelectedToGlobalCenter();
    void alignSelectedToGlobalVerticalCenter();
    void alignSelectedToGlobalHorizontalCenter();
    void alignSelectedToLocalCenter();
    void alignSelectedToLocalVerticalCenter();
    void alignSelectedToLocalHorizontalCenter();
    void selectPartAllById(QUuid partId);
    void addSelectNode(QUuid nodeId);
    void addSelectEdge(QUuid edgeId);
    void enableBackgroundBlur();
    void disableBackgroundBlur();
    void ikMove(QUuid endEffectorId, QVector3D target);
    void ikMoveReady();
    void setSelectedNodesBoneMark(BoneMark boneMark);
    void timeToRemoveDeferredNodesAndEdges();
    void switchSelectedXZ();
    void switchSelectedChainSide();
    void shortcutDelete();
    void shortcutAddMode();
    void shortcutUndo();
    void shortcutRedo();
    void shortcutXlock();
    void shortcutYlock();
    void shortcutZlock();
    void shortcutCut();
    void shortcutCopy();
    void shortcutPaste();
    void shortcutSave();
    void shortcutSelectMode();
    void shortcutDragMode();
    void shortcutZoomRenderedModelByMinus10();
    void shortcutZoomSelectedByMinus1();
    void shortcutZoomRenderedModelBy10();
    void shortcutZoomSelectedBy1();
    void shortcutRotateSelectedByMinus1();
    void shortcutRotateSelectedBy1();
    void shortcutMoveSelectedToLeft();
    void shortcutMoveSelectedToRight();
    void shortcutMoveSelectedToUp();
    void shortcutMoveSelectedToDown();
    void shortcutScaleSelectedByMinus1();
    void shortcutScaleSelectedBy1();
    void shortcutSwitchProfileOnSelected();
    void shortcutShowOrHideSelectedPart();
    void shortcutEnableOrDisableSelectedPart();
    void shortcutLockOrUnlockSelectedPart();
    void shortcutXmirrorOnOrOffSelectedPart();
    void shortcutSubdivedOrNotSelectedPart();
    void shortcutRoundEndOrNotSelectedPart();
    void shortcutWrapOrNotSelectedPart();
private slots:
    void turnaroundImageReady();
private:
    QPointF mouseEventScenePos(QMouseEvent *event);
    QPointF scenePosToUnified(QPointF pos);
    QPointF scenePosFromUnified(QPointF pos);
    float sceneRadiusToUnified(float radius);
    float sceneRadiusFromUnified(float radius);
    void updateTurnaround();
    void updateItems();
    void checkRangeSelection();
    void clearRangeSelection();
    void removeItem(QGraphicsItem *item);
    QVector2D centerOfNodeItemSet(const std::set<SkeletonGraphicsNodeItem *> &set);
    bool isSingleNodeSelected();
    void addItemToRangeSelection(QGraphicsItem *item);
    void removeItemFromRangeSelection(QGraphicsItem *item);
    void hoverPart(QUuid partId);
    void switchProfileOnRangeSelection();
    void setItemHoveredOnAllProfiles(QGraphicsItem *item, bool hovered);
    void alignSelectedToGlobal(bool alignToVerticalCenter, bool alignToHorizontalCenter);
    void alignSelectedToLocal(bool alignToVerticalCenter, bool alignToHorizontalCenter);
    void rotateItems(const std::set<SkeletonGraphicsNodeItem *> &nodeItems, int degree, QVector2D center);
    void rotateAllSideProfile(int degree);
private: //need initalize
    const SkeletonDocument *m_document;
    QGraphicsPixmapItem *m_backgroundItem;
    bool m_turnaroundChanged;
    TurnaroundLoader *m_turnaroundLoader;
    bool m_dragStarted;
    bool m_moveStarted;
    SkeletonGraphicsNodeItem *m_cursorNodeItem;
    SkeletonGraphicsEdgeItem *m_cursorEdgeItem;
    SkeletonGraphicsNodeItem *m_addFromNodeItem;
    SkeletonGraphicsNodeItem *m_hoveredNodeItem;
    SkeletonGraphicsEdgeItem *m_hoveredEdgeItem;
    float m_lastAddedX;
    float m_lastAddedY;
    float m_lastAddedZ;
    SkeletonGraphicsSelectionItem *m_selectionItem;
    bool m_rangeSelectionStarted;
    bool m_mouseEventFromSelf;
    bool m_moveHappened;
    int m_lastRot;
    SkeletonGraphicsOriginItem *m_mainOriginItem;
    SkeletonGraphicsOriginItem *m_sideOriginItem;
    SkeletonGraphicsOriginItem *m_hoveredOriginItem;
    SkeletonGraphicsOriginItem *m_checkedOriginItem;
    unsigned long long m_ikMoveUpdateVersion;
    SkeletonIkMover *m_ikMover;
    QTimer *m_deferredRemoveTimer;
    bool m_eventForwardingToModelWidget;
    ModelWidget *m_modelWidget;
    bool m_inTempDragMode;
    SkeletonDocumentEditMode m_modeBeforeEnterTempDragMode;
    bool m_nodePositionModifyOnly;
private:
    QVector3D m_ikMoveTarget;
    QUuid m_ikMoveEndEffectorId;
    std::set<QGraphicsItem *> m_rangeSelectionSet;
    QPoint m_lastGlobalPos;
    QPointF m_lastScenePos;
    QPointF m_rangeSelectionStartPos;
    QUuid m_lastCheckedPart;
    std::set<QUuid> m_deferredRemoveNodeIds;
    std::set<QUuid> m_deferredRemoveEdgeIds;
};

class SkeletonGraphicsContainerWidget : public QWidget
{
    Q_OBJECT
signals:
    void containerSizeChanged(QSize size);
public:
    SkeletonGraphicsContainerWidget() :
        m_graphicsWidget(nullptr)
    {
    }
    void resizeEvent(QResizeEvent *event) override
    {
        if (m_graphicsWidget && m_graphicsWidget->size() != event->size())
            emit containerSizeChanged(event->size());
    }
    void setGraphicsWidget(SkeletonGraphicsWidget *graphicsWidget)
    {
        m_graphicsWidget = graphicsWidget;
    }
private:
    SkeletonGraphicsWidget *m_graphicsWidget;
};

#endif
