#ifndef SNIPPETITEM_H
#define SNIPPETITEM_H

#include <QGraphicsItemGroup>
#include <QVector>

class QGraphicsRectItem;
class Snippet;

class SnippetItem : public QGraphicsItemGroup
{
public:
    SnippetItem(Snippet *snip, qreal units_per_ms, QGraphicsItem *parent = nullptr);
    void changeUnitsPerMs(qreal units_per_ms);
    Snippet *snippet() { return snip; }

public slots:
    void setHighlight(bool);
    // Should be called on changes to the snippet.
    void update();

private:
    QGraphicsRectItem *rect;
    QVector<QGraphicsLineItem*> key_frames;
    Snippet *snip;
    qreal units_per_ms;

    void setRect();
};

#endif // SNIPPETITEM_H
