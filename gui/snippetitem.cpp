#include "snippetitem.h"

#include "snippet.h"

#include <QCursor>
#include <QDebug>
#include <QGraphicsRectItem>
#include <QPen>

SnippetItem::SnippetItem(Snippet *snip, qreal units_per_ms, QGraphicsItem *parent) : QGraphicsItemGroup(parent)
{
    this->snip = snip;

    rect = new QGraphicsRectItem;
    changeUnitsPerMs(units_per_ms);
    rect->setPen(Qt::NoPen);
    rect->setBrush(QBrush(Qt::yellow));
    addToGroup(rect);

    update();
}

void SnippetItem::setRect()
{
    qreal x = units_per_ms * snip->startTime();
    qreal w = units_per_ms * snip->endTime() - x;
    // Roughly speaking, we want the snippet boxes to have height 1, but we hack in a
    // little padding so that they go from y coord 0.1 to 0.9.
    rect->setRect(x, 0.1, w, 0.8);
}

void
SnippetItem::changeUnitsPerMs(qreal units_per_ms)
{
    this->units_per_ms = units_per_ms;
    setRect();
}

void
SnippetItem::setHighlight(bool highlighted)
{
    auto color = highlighted ? Qt::darkYellow : Qt::yellow;
    rect->setBrush(QBrush(color));
}

void
SnippetItem::update()
{
    qDebug() << "updating snippet" << snip->keyTimes();
    setRect();

    QPen pen(Qt::black);
    pen.setWidth(0);

    for (auto line: key_frames) {
        delete line;
    }
    key_frames.clear();

    for (auto key_time: snip->keyTimes()) {
        QGraphicsLineItem *line = new QGraphicsLineItem;
        line->setPen(pen);
        line->setLine(units_per_ms * key_time, 0.1, units_per_ms * key_time, 0.9);
        key_frames.push_back(line);
        addToGroup(line);
    }
}
