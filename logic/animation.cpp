#include "animation.h"

#include "changingpath.h"
#include "snippet.h"

#include <QDebug>
#include <QPainterPath>

Animation::Animation(QObject *parent) : QObject(parent)
{

}

void Animation::addSnippet(Snippet *s, qint64 startTime)
{
    snippets.push_back(s);
    snippetStartTimes.push_back(startTime);

    emit snippetAdded(s, startTime);
}

QVector<RenderedPath>
Animation::updatedPaths(qint64 prev_t, qint64 cur_t) const
{
    QVector<RenderedPath> ret;

    for (int i = 0; i < snippets.length(); i++) {
        QVector<RenderedPath> v = snippets[i]->changedPaths(prev_t - snippetStartTimes[i], cur_t - snippetStartTimes[i]);
        ret.append(std::move(v));
    }

    return ret;
}

qint64 Animation::endTime() const
{
    // TODO: this is inefficient
    qint64 ret = 0;
    for (int i = 0; i < snippets.length(); i++) {
        ret = std::max(ret, snippetStartTimes[i] + snippets[i]->endTime());
    }
    return ret;
}
