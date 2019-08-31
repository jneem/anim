#include "snippet.h"

#include "changingpath.h"

#include <algorithm>
#include <cmath>
#include <QDebug>
#include <QPointF>
#include <QVector>

struct cmp_end {
    inline bool operator()(const ChangingPath *left, const ChangingPath *right) {
        return left->endTime() < right->endTime();
    }
};

Snippet::Snippet(QVector<ChangingPath *> &paths, qint64 start_time, qint64 end_time, QObject *parent) : QObject(parent)
{
    for (ChangingPath *p: paths) {
        p->setParent(this);
    }
    this->paths = paths;

    std::sort(this->paths.begin(), this->paths.end(), cmp_end());

    // We start out with up to four "keyframes": the start time, the end time, and the times of the first and last actual drawing.
    lerp_times.push_back(start_time);
    if (!paths.empty()) {
        qint64 first_draw_time = paths.first()->startTime();
        qint64 last_draw_time = paths.last()->endTime();
        if (first_draw_time > start_time) {
            lerp_times.push_back(first_draw_time);
        }
        if (last_draw_time < end_time) {
            lerp_times.push_back(last_draw_time);
        }
    }
    lerp_times.push_back(end_time);
    lerp_values = lerp_times;
}

struct find_end {
    inline bool operator()(const qint64 &time, const ChangingPath *path) {
        return time < path->endTime();
    }
};

QVector<RenderedPath> Snippet::changedPaths(qint64 prev_t, qint64 cur_t) const
{
    prev_t = unlerp(prev_t);
    cur_t = unlerp(cur_t);

    qint64 start = std::min(prev_t, cur_t);
    qint64 end = std::max(prev_t, cur_t);
    qint64 my_end_time = endTime();


    QVector<RenderedPath> ret;
    if (cur_t > my_end_time) {
        // Mark all paths as empty.
        for (auto p: paths) {
            ret.push_back(RenderedPath { p, QPainterPath() });
        }
        return ret;
    } else if (prev_t > my_end_time) {
        // At the previous time, nothing was visible. So we need to refresh everything.
        start = startTime();
    }

    // We are only interested in paths that finish after time `start`.
    auto i = std::upper_bound(paths.begin(), paths.end(), start, find_end());

    for(; i != paths.end(); i++) {
        // this isn't quite right.
        if ((*i)->startTime() < end) {
            auto painted = (*i)->toPainterPath(cur_t);
            ret.push_back(RenderedPath { *i, painted });
        }
    }
    return ret;
}

// TODO: document this, and explain why the return value is a pair.
std::pair<qint64, qint64>
Snippet::lerp_interval(qint64 time, const QVector<qint64> &from, const QVector<qint64> &to) const
{
    if (time >= from.last()) {
        qint64 ret = to.last() + (time - from.last());
        return std::make_pair(ret, ret);
    } else if (time <= from.first()) {
        qint64 ret = to.first() + (time - from.first());
        return std::make_pair(ret, ret);
    }

    auto iter = std::lower_bound(from.begin(), from.end(), time);
    auto i = static_cast<int>(iter - from.begin());
    if (from[i] > time) {
        i -= 1;
    }
    // Now we are guaranteed that time is between from[i] and from[i+1].

    if (from[i] == from[i+1]) {
        return std::make_pair(to[i], to[i+1]);
    }
    qreal ratio = static_cast<qreal>(time - from[i]) / static_cast<qreal>(from[i+1] - from[i]);
    auto ret = static_cast<qint64>(std::round((1-ratio)* to[i] + ratio * to[i+1]));
    return std::make_pair(ret, ret);
}

qint64 Snippet::lerp(qint64 time) const
{
    return lerp_interval(time, lerp_times, lerp_values).first;
}

qint64 Snippet::unlerp(qint64 time) const
{
    return lerp_interval(time, lerp_values, lerp_times).first;
}

// Returns the last index in times containing a value less than or equal to t. Note that this might be -1,
// if every value in times is greater than t.
// Assumes times is sorted.
int last_le(qint64 t, const QVector<qint64> &times)
{
    auto iter = std::lower_bound(times.constBegin(), times.constEnd(), t);
    int i = static_cast<int>(iter - times.constBegin());
    if (i >= times.length() || times[i] > t) {
        return i - 1;
    } else {
        return i;
    }
}

int first_gt(qint64 t, const QVector<qint64> &times)
{
    auto iter = std::upper_bound(times.constBegin(), times.constEnd(), t);
    return static_cast<int>(iter - times.constBegin());
}

void relerp_interval(QVector<qint64> *times, int start_idx, int end_idx, qint64 old_start_time, qint64 old_end_time)
{
    qint64 new_start_time = (*times)[start_idx];
    qint64 new_end_time = (*times)[end_idx];

    for (int i = start_idx + 1; i < end_idx; i++) {
        qreal ratio = static_cast<qreal>((*times)[i] - old_start_time) / static_cast<qreal>(old_end_time - old_start_time);
        (*times)[i] = static_cast<qint64>(std::round((1 - ratio) * new_start_time + ratio * new_end_time));
    }
}

void Snippet::addLerp(qint64 old_time, qint64 new_time)
{
    qint64 old_time_local = unlerp(old_time);

    int after_old_idx = first_gt(old_time_local, lerp_times);
    int new_idx;
    // If the old time was already mapped somewhere, remap it. Otherwise, add a new mapping.
    if (after_old_idx > 0 && lerp_times[after_old_idx - 1] == old_time_local) {
        new_idx = after_old_idx - 1;
        lerp_values[new_idx] = new_time;
    } else {
        new_idx = after_old_idx;
        lerp_times.insert(after_old_idx, old_time_local);
        lerp_values.insert(after_old_idx, new_time);
    }

    Q_ASSERT(new_time == lerp_values[new_idx]);
    // Ensure that lerp_values is still sorted.
    if (new_idx + 1 < lerp_values.length() && new_time > lerp_values[new_idx + 1]) {
        // anchor_idx is the index of the first time that we don't have to change. Everything between
        // new_idx and anchor_idx (exclusive) will be modified to remain sorted.
        int anchor_idx = first_gt(new_time, lerp_values);

        if (anchor_idx == lerp_values.length()) {
            // If *none* of the lerp_values are bigger than new_time, we just translate everything.
            for (int i = new_idx + 1; i < lerp_values.length(); i++) {
                lerp_values[i] += old_time - new_time;
            }
        } else {
            relerp_interval(&lerp_values, new_idx, anchor_idx, old_time, lerp_values[anchor_idx]);
        }
    } else if (new_idx > 0 && new_time < lerp_values[new_idx - 1]) {
        // The case where the new time happens before the previous time is basically the same,
        // but backwards.
        int anchor_idx = last_le(new_time, lerp_values);
        if (anchor_idx < 0) {
            for (int i = 0; i < new_idx; i++) {
                lerp_values[i] = std::max(0LL, lerp_values[i] + old_time - new_time);
            }
        } else {
            relerp_interval(&lerp_values, anchor_idx, new_idx, lerp_values[anchor_idx], old_time);
        }
    }

    // The cast that the

    emit changed();
}

const QVector<qint64>&
Snippet::keyTimes()
{
    return lerp_values;
}
