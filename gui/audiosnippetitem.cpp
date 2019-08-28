#include "audiosnippetitem.h"

#include "audiosnippet.h"

#include <QBrush>
#include <QDebug>
#include <QGraphicsRectItem>
#include <QPen>

AudioSnippetItem::AudioSnippetItem(AudioSnippet *snip, qreal units_per_ms, QGraphicsItem *parent) : QGraphicsItemGroup(parent)
{
    this->snip = snip;
    this->units_per_ms = units_per_ms;

    rect = new QGraphicsRectItem(this);
    waveform = new QGraphicsPathItem(this);
    addToGroup(rect);
    addToGroup(waveform);
    rect->setPen(Qt::NoPen);
    rect->setBrush(QBrush(Qt::cyan));

    setRect();
    setWaveform();

    qDebug() << "adding audio snippet from" << snip->startTime() << "to" << snip->endTime();
}

void
AudioSnippetItem::setRect()
{
    qreal x = units_per_ms * snip->startTime();
    qreal w = units_per_ms * snip->endTime() - x;
    rect->setRect(x, 0.1, w, 0.8);
}

void AudioSnippetItem::setWaveform()
{
    // TODO
}
