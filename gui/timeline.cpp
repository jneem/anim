#include "timeline.h"

#include "snippet.h"
#include "snippetitem.h"
#include "timelinelayout.h"

#include <QDebug>
#include <QGraphicsItemGroup>
#include <QGraphicsLineItem>
#include <QResizeEvent>

Timeline::Timeline(QWidget *parent) : QGraphicsView(parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHint(QPainter::Antialiasing);

    scene = new QGraphicsScene(this);
    setScene(scene);

    // This has nothing to do with the logical units: we fix a physical height of 150px.
    setFixedHeight(150);

    QColor color;
    color.setNamedColor("darkgreen");
    QPen pen(color);
    pen.setWidth(0);

    QGraphicsLineItem *curs_line = scene->addLine(QLineF(0, 0, 0, 10), pen);
    QPolygonF triangle;
    triangle << QPointF(-0.5, 0) << QPointF(0.5, 0) <<  QPointF(0, 0.5);
    QGraphicsPolygonItem *curs_triangle = scene->addPolygon(triangle, pen, QBrush(color));
    cursor = scene->createItemGroup({curs_line, curs_triangle});
    cursor->setZValue(1.0); // Draw the cursor on top (0.0 is the default Z value).
}

void Timeline::updateTime(qint64 t)
{
    qreal x = t * units_per_ms;
    //qDebug() << "updateTime" << t << x;
    cursor->setTransform(QTransform::fromTranslate(x, 0));
}

// TODO: is this obsolete now?
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

void Timeline::mousePressEvent(QMouseEvent *event)
{
    event->accept();
    mouse_press_location = event->pos();
    dragging_cursor = cursor->contains(cursor->mapFromScene(mapToScene(event->pos())));
    if (dragging_cursor)
        qDebug() << "start dragging";

}

void Timeline::mouseMoveEvent(QMouseEvent *event)
{
    event->accept();
    if (playing_or_recording) {
        return;
    }

    if (dragging_cursor) {
        updateTimeFromPos(event->pos());
    }
}

void Timeline::updateTimeFromPos(const QPoint &pos)
{
    qreal scene_x = mapToScene(pos).x();
    scene_x = std::max(scene_x, 0.0);
    qint64 t = static_cast<qint64>(scene_x / units_per_ms);
    updateTime(t);
    emit timeWarped(t);
}

void Timeline::mouseReleaseEvent(QMouseEvent *event)
{
    event->accept();
    dragging_cursor = false;
    if (playing_or_recording) {
        return;
    }

    if (mouse_press_location == event->pos()) {
        // This counts as a mouse click. Move the cursor to the new time.
        updateTimeFromPos(event->pos());
    }
}

void Timeline::startPlaying()
{
    playing_or_recording = true;
}

void Timeline::stopPlaying()
{
    playing_or_recording = false;
}

void Timeline::startRecording()
{
    playing_or_recording = true;
}

void Timeline::stopRecording()
{
    playing_or_recording = false;
}

// When a snippet is added to the animation, we need to create a corresponding item so that it can be seen in the timeline.
void Timeline::addSnippet(Snippet *snip, qint64 start_time)
{
    SnippetItem *item = new SnippetItem(snip, units_per_ms);
    snippet_to_item.insert(snip, item);
    snippet_to_start_time.insert(snip, start_time);
    scene->addItem(item);

    qDebug() << "adding snippet, start time" << start_time << ", length" << snip->endTime();

    // Recalculate the vertical layout of all the snippet items. (This could probably be done more incrementally.)
    QList<TimelineElement<Snippet*>> elts;
    for (auto it = snippet_to_start_time.begin(); it != snippet_to_start_time.end(); it++) {
        Snippet *snippet = it.key();
        qint64 start = it.value();
        elts.push_back(TimelineElement<Snippet*>{ start, start + snippet->endTime(), snippet});
    }
    QMap<Snippet*, int> depth = layoutTimeline(elts);

    for (auto it = snippet_to_item.begin(); it != snippet_to_item.end(); it++) {
        Snippet *snippet = it.key();
        qint64 start_time = snippet_to_start_time.value(snippet);
        it.value()->setTransform(QTransform::fromTranslate(start_time * units_per_ms, depth.value(snippet)));
    }
}

void Timeline::removeSnippet(Snippet *snip)
{
    auto item = snippet_to_item.take(snip);
    snippet_to_start_time.remove(snip);
    scene->removeItem(item);
}
