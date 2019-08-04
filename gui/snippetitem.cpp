#include "snippetitem.h"

#include "snippet.h"

#include <QDebug>
#include <QGraphicsRectItem>
#include <QPen>

SnippetItem::SnippetItem(Snippet *snip, qreal units_per_ms, QGraphicsItem *parent) : QGraphicsItemGroup(parent)
{
    snippet = snip;

    QPen pen(Qt::black);
    pen.setWidth(0);

    rect = new QGraphicsRectItem;
    changeUnitsPerMs(units_per_ms);
    rect->setPen(pen);
    rect->setBrush(QBrush(Qt::yellow));
    addToGroup(rect);
}

void
SnippetItem::changeUnitsPerMs(qreal units_per_ms)
{
    // Roughly speaking, we want the snippet boxes to have height 1, but we hack in a
    // little padding so that they go from y coord 0.1 to 0.9.
    rect->setRect(0, 0.1, units_per_ms * snippet->endTime(), 0.8);
}
