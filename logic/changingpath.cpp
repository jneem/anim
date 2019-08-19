#include "changingpath.h"

#include <QMultiMap>
#include <QPainterPath>
#include <QPointF>

ChangingPath::ChangingPath(QObject *parent) : QObject(parent)
{
}

void ChangingPath::startAt(qint64 time, const QPointF &pos)
{
    points.clear();
    points.push_back(pos);
    times.push_back(time);
}

void ChangingPath::lineTo(qint64 time, const QPointF &pos)
{
    Q_ASSERT(time >= times.last());
    points.push_back(pos);
    times.push_back(time);
}

QPainterPath ChangingPath::toPainterPath(qint64 time) const
{
    QPainterPath ret;
    int i = 0;
    ret.moveTo(points[i]);
    i++;

    for(; i < points.length() && times[i] <= time; i++) {
        ret.lineTo(points[i]);
    }
    return ret;
}

QPainterPath ChangingPath::toPainterPath() const
{
    return toPainterPath(times.last());
}

qint64 ChangingPath::startTime() const
{
    return times[0];
}

qint64 ChangingPath::endTime() const
{
    return times.last();
}

qreal area(const QPointF &a, const QPointF &b, const QPointF &c) {
    return abs((a.x() * (b.y() - c.y()) + b.x() * (c.y() - a.y()) + c.x() * (a.y() - b.y())) / 2);
}

void ChangingPath::simplify()
{
    // We'll use Visvanlingam's algorithm: any triangle between 3 adjacent
    // vertices that has a small area will be removed. In order to avoid
    // repeatedly deleting nodes in the middle of the path, we first leave the
    // path unmodified and simply keep track of which elements we intend to
    // delete.
    const qreal MIN_AREA = 1;

    QVector<bool> live;
    // If i is a live index, prevLiveIdx[i] will be the previous live index
    // and nextLiveIdx[i] will be the next live index.
    QVector<int> prevLiveIdx;
    QVector<int> nextLiveIdx;

    // This is a map from triangle areas to indices. (The index is the middle vertex of the triangle.)
    QMultiMap<qreal, int> areas;
    // This is the inverse map of `areas`. Importantly, the values here are up-to-date, whereas the values
    // in `areas` could be stale, because we don't delete the old values when recalculating.
    QVector<qreal> fresh_areas;

    fresh_areas.push_back(0);
    for (int i=0; i < points.size(); i++) {
        prevLiveIdx.push_back(i-1);
        nextLiveIdx.push_back(i+1);
        live.push_back(true);

        if (i > 0 && i + 1 < points.size()) {
            qreal a = area(points[i-1], points[i], points[i+1]);
            fresh_areas.push_back(a);
            areas.insert(a, i);
        }
    }

    while (!areas.isEmpty()) {
        qreal k = areas.firstKey();
        int i = areas.take(k);
        if (fresh_areas[i] != k) {
            // This area is stale, ignore it.
            continue;
        }

        if (k <= MIN_AREA) {
            live[i] = false;
            int prev = prevLiveIdx[i];
            int next = nextLiveIdx[i];
            nextLiveIdx[prev] = next;
            nextLiveIdx[next] = prev;

            // Update the areas of the two neighboring triangles. The fact that we just removed a triangle
            // implies that prev and next are both live indices.
            int prevprev = prevLiveIdx[prev];
            int nextnext = nextLiveIdx[next];
            if (prevprev >= 0) {
                qreal a = area(points[prevprev], points[prev], points[next]);
                areas.insert(a, prev);
                fresh_areas[prev] = a;
            } else {
                fresh_areas[prev] = qInf();
            }

            if (nextnext < points.size()) {
                qreal a = area(points[prev], points[next], points[nextnext]);
                areas.insert(a, next);
                fresh_areas[next] = a;
            } else {
                fresh_areas[next] = qInf();
            }
        }
    }

    // TODO: actually delete the unneeded entries
}
