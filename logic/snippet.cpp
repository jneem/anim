#include "snippet.h"

#include "changingpath.h"

#include <algorithm>
#include <QDebug>
#include <QPointF>
#include <QVector>

struct cmp_end {
    inline bool operator()(const ChangingPath *left, const ChangingPath *right) {
        return left->endTime() < right->endTime();
    }
};

Snippet::Snippet(QVector<ChangingPath *> &paths, qint64 finishTime, QObject *parent) : QObject(parent)
{
    for (ChangingPath *p: paths) {
        p->setParent(this);
    }
    this->paths = paths;

    std::sort(this->paths.begin(), this->paths.end(), cmp_end());

    lerpTimes.push_back(0);
    lerpTimes.push_back(finishTime);
    lerpValues = lerpTimes;
}

struct find_end {
    inline bool operator()(const qint64 &time, const ChangingPath *path) {
        return time < path->endTime();
    }
};

QVector<RenderedPath> Snippet::changedPaths(qint64 start, qint64 end) const
{
    start = unlerp(start);
    end = unlerp(end);

    QVector<RenderedPath> ret;

    // We are only interested in paths that finish after time `start`.
    auto i = std::upper_bound(paths.begin(), paths.end(), start, find_end());

    for(; i != paths.end(); i++) {
        if ((*i)->startTime() < end) {
            auto painted = (*i)->toPainterPath(end);
            ret.push_back(RenderedPath { *i, painted });
        }
    }
    return ret;
}

qint64 Snippet::lerp(qint64 time) const
{
    auto iter = std::lower_bound(lerpTimes.begin(), lerpTimes.end(), time);
    auto i = static_cast<int>(iter - lerpTimes.begin());
    qreal ratio = static_cast<qreal>(time - lerpTimes[i]) / static_cast<qreal>(lerpTimes[i+1] - lerpTimes[i]);
    return static_cast<qint64>((1-ratio) * lerpValues[i] + ratio * lerpValues[i+1]);
}

qint64 Snippet::unlerp(qint64 time) const
{
    auto iter = std::upper_bound(lerpValues.begin(), lerpValues.end(), time) - 1;
    auto i = static_cast<int>(iter - lerpValues.begin());
    if (i + 1 >= lerpValues.length()) {
        return lerpTimes.last();
    } else if (i < 0) {
        return lerpTimes.first();
    }
    qreal ratio = static_cast<qreal>(time - lerpValues[i]) / static_cast<qreal>(lerpValues[i+1] - lerpValues[i]);
    return static_cast<qint64>((1-ratio) * lerpTimes[i] + ratio * lerpTimes[i+1]);
}
