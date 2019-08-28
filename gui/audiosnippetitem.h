#ifndef AUDIOSNIPPETITEM_H
#define AUDIOSNIPPETITEM_H

#include <QGraphicsItemGroup>

class QGraphicsRectItem;
class AudioSnippet;

class AudioSnippetItem : public QGraphicsItemGroup
{
public:
    AudioSnippetItem(AudioSnippet *snip, qreal units_per_ms, QGraphicsItem *parent = nullptr);
    AudioSnippet *snippet() { return snip; }

private:
    QGraphicsRectItem *rect;
    QGraphicsPathItem *waveform;
    AudioSnippet *snip;
    qreal units_per_ms;

    void setRect();
    void setWaveform();
};

#endif // AUDIOSNIPPETITEM_H
