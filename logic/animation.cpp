#include "animation.h"

#include "changingpath.h"
#include "snippet.h"

#include <QDebug>
#include <QPainterPath>

Animation::Animation(QObject *parent) : QObject(parent)
{

}

// After adding a snippet to an animation, you must never modify it again. Otherwise, updatedPaths might not be correct.
void Animation::addSnippet(Snippet *s)
{
    Q_ASSERT(!snippets.contains(s));
    snippets.insert(s);

    emit snippetAdded(s);
}

QVector<RenderedPath>
Animation::updatedPaths(qint64 prev_t, qint64 cur_t)
{
    QVector<RenderedPath> ret;

    // TODO: we're looping over all snippets here, but we could be more efficient.
    for (Snippet *s : snippets) {
        QVector<RenderedPath> v;
        if (dirty_snippets.contains(s)) {
            v = s->changedPaths(s->startTime(), s->endTime());
        } else {
            v = s->changedPaths(prev_t, cur_t);

        }
        ret.append(std::move(v));
    }
    dirty_snippets.clear();

    return ret;
}

qint64 Animation::endTime() const
{
    // TODO: this is inefficient
    qint64 ret = 0;
    for (Snippet *s : snippets) {
        ret = std::max(ret, s->endTime());
    }
    return ret;
}

void Animation::warpSnippet(Snippet *s, qint64 old_time, qint64 new_time)
{
    Q_ASSERT(snippets.contains(s));
    dirty_snippets.insert(s);
    s->addLerp(old_time, new_time);
    emit snippetChanged(s);
}
