#ifndef SNIPPET_H
#define SNIPPET_H

#include <QElapsedTimer>
#include <QObject>
#include <QVector>

class ChangingPath;
class QPointF;
class RenderedPath;

class Snippet : public QObject
{
    Q_OBJECT
public:
    explicit Snippet(QVector<ChangingPath*> &paths, qint64 start_time, qint64 finish_time, QObject *parent = nullptr);

    QVector<RenderedPath> changedPaths(qint64 start, qint64 end) const;
    const QVector<ChangingPath*> &allPaths() const { return paths; }
    qint64 startTime() const { return lerp_values.first(); }
    qint64 endTime() const { return lerp_values.last(); }

    // TODO: docme
    void addLerp(qint64 old_time, qint64 new_time);

    const QVector<qint64> &keyTimes();

    qint64 lerp(qint64 time) const;
    qint64 unlerp(qint64 time) const;
signals:
    void changed();

public slots:

private:
    std::pair<qint64, qint64> lerp_interval(qint64 time, const QVector<qint64> &from, const QVector<qint64> &to) const;

    // The list of paths, ordered by their ending time.
    QVector<ChangingPath*> paths;

    // The (ordered) list of time points marking the boundaries of interpolation regions.
    // This always has length at least 2, with the first value being zero and the last value
    // being the end of this snippet.
    QVector<qint64> lerp_times;

    // The new times. That is, lerpTimes[i] will actually happen at time lerpValues[i], and
    // the times in between lerpTimes[i] and lerpTimes[i+1] will be linearly interpolated
    // to happen between lerpValues[i] and lerpValues[i+1]. In particular, lerpValues and
    // lerpTimes have the same length.
    QVector<qint64> lerp_values;
};

#endif // SNIPPET_H
