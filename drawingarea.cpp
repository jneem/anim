#include "drawingarea.h"

#include "changingpath.h"

#include <QDebug>
#include <QEvent>
#include <QGraphicsPathItem>
#include <QGraphicsView>
#include <QGridLayout>
#include <QPainterPath>
#include <QTabletEvent>
#include <QPushButton>

GraphicsView::GraphicsView(QWidget *parent) : QGraphicsView(parent)
{
    curPathItem = nullptr;
    path = new ChangingPath(this);
    setRenderHint(QPainter::Antialiasing);
}

void GraphicsView::tabletEvent(QTabletEvent *event)
{
    event->accept();
    qDebug() << "event type" << event->type()
        << "pressure" << event->pressure()
        << "position" << event->posF();

    // TODO: tablet events support high-resolution positions, but I don't see how to map them to the scene.
    QPointF pos = mapToScene(event->pos());
    if (event->type() == QEvent::TabletPress) {
        if (curPathItem) {
            finishPath(pos);
        }

        path->startAt(pos);
        curPathItem = new QGraphicsPathItem();
        scene()->addItem(curPathItem);
    } else if (event->type() == QEvent::TabletMove) {
        path->lineTo(pos);
        if (curPathItem) {
            curPathItem->setPath(path->toPainterPath());
        }
    } else if (event->type() == QEvent::TabletRelease) {
        if (curPathItem) {
            finishPath(pos);
        }
    }
}

void GraphicsView::finishPath(const QPointF &pos) {
    path->lineTo(pos);
    curPathItem->setPath(path->toPainterPath());
    curPathItem = nullptr;
}

DrawingArea::DrawingArea(QWidget *parent) : QWidget(parent)
{
    scene = new QGraphicsScene(this);
    view = new GraphicsView(this);
    view->setScene(scene);
    view->setSceneRect(0, 0, 640, 480);

    QGraphicsItem *item = new QGraphicsRectItem(0, 0, 640, 480);
    scene->addItem(item);

    clearButton = new QPushButton("Clear", this);
    clearButton->setGeometry(10, 10, 80, 30);

    QGridLayout *topLayout = new QGridLayout;
    topLayout->addWidget(view);
    topLayout->addWidget(clearButton);
    setLayout(topLayout);
}
