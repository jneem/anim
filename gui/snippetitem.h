#ifndef SNIPPETITEM_H
#define SNIPPETITEM_H

#include <QGraphicsItemGroup>

class QGraphicsRectItem;
class Snippet;

class SnippetItem : public QGraphicsItemGroup
{
public:
    SnippetItem(Snippet *snip, qreal units_per_ms, QGraphicsItem *parent = nullptr);
    void changeUnitsPerMs(qreal units_per_ms);

private:
    QGraphicsRectItem *rect;
    Snippet *snippet;
};

#endif // SNIPPETITEM_H
