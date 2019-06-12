#include "timeline.h"

#include <QDebug>
#include <QGraphicsLineItem>
#include <QResizeEvent>

Timeline::Timeline(QWidget *parent) : QGraphicsView(parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHint(QPainter::Antialiasing);

    scene = new QGraphicsScene(this);
    setScene(scene);

    // This has nothing to do with the logical units: we fix a physical height of 150px.
    setFixedHeight(150);

    QColor color;
    color.setNamedColor("darkgreen");
    QPen pen(color);
    pen.setWidthF(0.1);
    cursor = new QGraphicsLineItem(0, 0, 0, 10);
    cursor->setPen(pen);
    scene->addItem(cursor);
}

void Timeline::updateTime(qint64 t)
{
    qreal x = 100.0 * static_cast<qreal>(t) / static_cast<qreal>(maxTime);
    qDebug() << "updateTime" << t << x;
    cursor->setLine(x, 0, x, 20);
}

void Timeline::updateTimeBounds(qint64 t)
{
    maxTime = std::max(maxTime, t);
}

void Timeline::resizeEvent(QResizeEvent *event)
{
    qDebug() << "resize" << event->size();
    QRectF sceneRect(-1, 0, 101, 10);
    setScene(scene);
    setSceneRect(sceneRect);
    centerOn(sceneRect.center());
    // TODO: keep the scale updated on resizing.
    setTransform(QTransform());
    scale(width() / sceneRect.width(), height() / sceneRect.height());
    qDebug() << "width" << width() << "height" << height();
}
