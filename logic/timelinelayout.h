#ifndef TIMELINELAYOUT_H
#define TIMELINELAYOUT_H

#include <QMap>

template<typename T>
struct TimelineElement {
    qint64 start;
    qint64 end;
    T data;
};

template<typename T>
struct Event {
    qint64 time;
    bool start_span;
    T data;

    bool operator<(const Event &other) {
        return time < other.time || (time == other.time && start_span > other.start_span);
    }
};

// Note: the Ts need to be unique for this to be correct.
template<typename T>
QMap<T, int> layoutTimeline(const QList<TimelineElement<T>> &elts) {
    QList<Event<T>> events;
    for (TimelineElement<T> elt: elts) {
        events.push_back(Event<T> { elt.start, true, elt.data });
        events.push_back(Event<T> { elt.end, false, elt.data });
    }
    std::sort(events.begin(), events.end());

    QMap<T, int> ret;
    int nextUnusedPos = 0;
    // This is just treated as an ordered set of "holes": that is, positions between 0
    // (inclusive) and nextUnusedPos (exclusive) that are not currently being used.
    QMap<int, bool> holes;
    for (Event<T> ev: events) {
        if (ev.start_span) {
            // Check if there is a hole available. If so, use it; if not, get the next unused position.
            int pos = 0;
            if (!holes.empty()) {
                pos = holes.firstKey();
                holes.remove(pos);
            } else {
                pos = nextUnusedPos;
                nextUnusedPos++;
            }
            ret.insert(ev.data, pos);
        } else {
            // The span is over, so mark the position as unused.
            int pos = ret.value(ev.data);
            holes.insert(pos, true);
        }
    }

    return ret;
}

#endif // TIMELINELAYOUT_H
