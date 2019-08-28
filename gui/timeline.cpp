#include "timeline.h"

#include "audiosnippet.h"
#include "audiosnippetitem.h"
#include "snippet.h"
#include "snippetitem.h"
#include "timelinelayout.h"

#include <QDebug>
#include <QGraphicsItemGroup>
#include <QGraphicsLineItem>
#include <QResizeEvent>

// TODO: handle the case where there are more than 10 layers

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
    cursor->setTransform(QTransform::fromTranslate(x, 0));
}

void Timeline::resizeEvent(QResizeEvent *event)
{
    qDebug() << "resize" << event->size();
    QRectF sceneRect(-1, -0.5, 101, 10.5);
    setScene(scene);
    setSceneRect(sceneRect);
    centerOn(sceneRect.center());
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

void Timeline::setMark(qint64 time)
{
    if (!mark) {
        QPen pen(Qt::blue);
        pen.setStyle(Qt::DotLine);
        pen.setWidth(0);
        mark = scene->addLine(0, 0, 0, 10, pen);
    }
    mark->setTransform(QTransform::fromTranslate(time * units_per_ms, 0));
}

void Timeline::mouseReleaseEvent(QMouseEvent *event)
{
    event->accept();
    dragging_cursor = false;
    if (playing_or_recording) {
        return;
    }

    if (mouse_press_location == event->pos()) {
        // This counts as a mouse click. If we're clicking on a snippet, set it as highlighted.
        // Otherwise, warp the cursor.
        // NOTE: it might be nicer to implement the snippet focusing by doing mouse handling on
        // the SnippetItem -- but then we have to re-implement the click detection there...
        auto item = itemAt(event->pos());
        while (item) {
            auto *snip_item = dynamic_cast<SnippetItem*>(item);
            if (snip_item) {
                emit highlightedSnippet(snip_item->snippet());
                return;
            }
            item = item->parentItem();
        }

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
void Timeline::addSnippet(Snippet *snip)
{
    SnippetItem *item = new SnippetItem(snip, units_per_ms);
    snippet_to_item.insert(snip, item);
    scene->addItem(item);

    relayout();
    emit highlightedSnippet(snip);
}

void Timeline::relayout()
{
    // Recalculate the vertical layout of all the snippet items. (This could probably be done more incrementally.)
    QList<TimelineElement<Snippet*>> elts;
    for (auto it = snippet_to_item.begin(); it != snippet_to_item.end(); it++) {
        Snippet *snippet = it.key();
        elts.push_back(TimelineElement<Snippet*>{ snippet->startTime(), snippet->endTime(), snippet });
    }

    QMap<Snippet*, int> depth = layoutTimeline(elts);
    for (auto it = snippet_to_item.begin(); it != snippet_to_item.end(); it++) {
        Snippet *snippet = it.key();
        it.value()->setTransform(QTransform::fromTranslate(0, depth.value(snippet)));
    }

    // Now repeat for the audio snippets.
    QList<TimelineElement<AudioSnippet*>> audio_elts;
    for (auto it = audio_snippet_to_item.begin(); it != audio_snippet_to_item.end(); it++) {
        AudioSnippet *snip = it.key();
        audio_elts.push_back(TimelineElement<AudioSnippet*>{ snip->startTime(), snip->endTime(), snip });
    }
    QMap<AudioSnippet*, int> audio_depth = layoutTimeline(audio_elts);
    for (auto it = audio_snippet_to_item.begin(); it != audio_snippet_to_item.end(); it++) {
        AudioSnippet *snip = it.key();
        it.value()->setTransform(QTransform::fromTranslate(0, 9 - audio_depth.value(snip)));
    }
}

void Timeline::updateSnippet(Snippet *snip)
{
    SnippetItem *item = snippet_to_item.value(snip);
    item->update();
    relayout();
}

void Timeline::removeSnippet(Snippet *snip)
{
    auto item = snippet_to_item.take(snip);
    scene->removeItem(item);

    if (item == highlighted) {
        highlighted = nullptr;
    }
}

void Timeline::highlightSnippet(Snippet *snip)
{
    if (highlighted) {
        highlighted->setHighlight(false);
    }
    highlighted = snippet_to_item.value(snip);
    if (highlighted) {
        highlighted->setHighlight(true);
    }
}

void Timeline::addAudioSnippet(AudioSnippet *snip)
{
    auto item = new AudioSnippetItem(snip, units_per_ms);
    scene->addItem(item);
    audio_snippet_to_item.insert(snip, item);
    relayout();
}
