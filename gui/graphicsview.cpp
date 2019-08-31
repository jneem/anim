#include "graphicsview.h"

#include "animation.h"
#include "changingpath.h"
#include "recordingsnippet.h"
#include "snippet.h"
#include "timeline.h"

#include <QDebug>
#include <QEvent>
#include <QGraphicsPathItem>
#include <QGraphicsView>
#include <QGridLayout>
#include <QGroupBox>
#include <QPainterPath>
#include <QTabletEvent>
#include <QTimer>
#include <QPushButton>
#include <QVBoxLayout>

GraphicsView::GraphicsView(QWidget *parent) : QGraphicsView(parent)
{
    QGraphicsScene *scene = new QGraphicsScene(this);
    setScene(scene);
    setSceneRect(0, 0, 640, 480);
    setAnimation(anim);
    setRenderHint(QPainter::Antialiasing);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QGraphicsItem *border = new QGraphicsRectItem(0, 0, 640, 480);
    scene->addItem(border);

    curPathItem = nullptr;
    recSnippet = nullptr;
    anim = nullptr;
}

void GraphicsView::tabletEvent(QTabletEvent *event)
{
    event->accept();

    // TODO: tablet events support high-resolution positions, but I don't see how to map them to the scene.
    if (recSnippet) {
        QPointF pos = mapToScene(event->pos());
        if (event->type() == QEvent::TabletPress) {
            // It seems to be possible to get two presses with no release in between.
            // In this case, we continue the previous path instead of starting a new one.
            if (!recSnippet->currentPath()) {
                recSnippet->startPath(pos);
                curPathItem = new QGraphicsPathItem();
                scene()->addItem(curPathItem);

                pathMap.insert(recSnippet->currentPath(), curPathItem);
                revPathMap.insert(curPathItem, recSnippet->currentPath());
            }
        } else if (event->type() == QEvent::TabletMove) {
            recSnippet->lineTo(pos);
            if (curPathItem) {
                curPathItem->setPath(recSnippet->currentPath()->toPainterPath());
            }
        } else if (event->type() == QEvent::TabletRelease) {
            if (curPathItem) {
                curPathItem->setPath(recSnippet->currentPath()->toPainterPath());
                curPathItem = nullptr;
            }
            recSnippet->finishPath(pos);
        }
    }
}

void GraphicsView::resizeEvent(QResizeEvent *)
{
    // Essentially, we want to display a 640x480 grid, but make it a little larger to allow for the border.
    QRectF sceneRect(-2, -2, 642, 482);
    setSceneRect(sceneRect);
    centerOn(sceneRect.center());
    setTransform(QTransform());

    qreal scalex = width() / sceneRect.width();
    qreal scaley = height() / sceneRect.height();
    qreal s = std::min(scalex, scaley);
    scale(s, s);
}

void GraphicsView::startRecording(qint64 time)
{
    qDebug() << "GraphicsView::startRecording";
    recSnippet = new RecordingSnippet(time, this);
    rec_start_time = time;
}

Snippet*
GraphicsView::stopRecording()
{
    qDebug() << "GraphicsView::stopRecording";
    if (recSnippet) {
        Snippet *snippet = recSnippet->finishRecording();
        delete recSnippet;
        recSnippet = nullptr;
        return snippet;
    }
    return nullptr;
}

void GraphicsView::update(qint64 prev_t, qint64 cur_t)
{
    auto pathsToUpdate = anim->updatedPaths(prev_t, cur_t);
    for (auto p: pathsToUpdate) {
        QGraphicsPathItem *item = pathMap.value(p.path);
        if (item) {
            item->setPath(p.rendered);
        } else {
            qDebug() << "failed to find item for path" << p.path;
        }
    }
}

void
GraphicsView::addSnippet(Snippet *snip, qint64 cur_time)
{
    auto all_paths = snip->allPaths();
    for (auto p: all_paths) {
        // There's a chance that we already have items for these paths, for example if we just recorded them.
        if (!pathMap.contains(p)) {
            auto item = new QGraphicsPathItem;
            scene()->addItem(item);
            pathMap.insert(p, item);
            revPathMap.insert(item, p);
        }
    }

    auto visible_paths = snip->changedPaths(snip->startTime(), cur_time);
    for (auto p: visible_paths) {
        auto item = pathMap.value(p.path);
        item->setPath(p.rendered);
    }
}

void
GraphicsView::removeSnippet(Snippet *snip)
{
    auto paths = snip->allPaths();
    for (auto p: paths) {
        QGraphicsPathItem *item = pathMap.take(p);
        scene()->removeItem(item);
        revPathMap.remove(item);
    }
}
