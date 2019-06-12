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
    explicit Snippet(QVector<ChangingPath*> &paths, qint64 finishTime, QObject *parent = nullptr);

    QVector<RenderedPath> changedPaths(qint64 start, qint64 end) const;
    qint64 endTime() const { return lerpValues.last(); }

signals:

public slots:

private:
    // The list of paths, ordered by their ending time.
    QVector<ChangingPath*> paths;

    // The (ordered) list of time points marking the boundaries of interpolation regions.
    // This always has length at least 2, with the first value being zero and the last value
    // being the end of this snippet.
    QVector<qint64> lerpTimes;

    // The new times. That is, lerpTimes[i] will actually happen at time lerpValues[i], and
    // the times in between lerpTimes[i] and lerpTimes[i+1] will be linearly interpolated
    // to happen between lerpValues[i] and lerpValues[i+1]. In particular, lerpValues and
    // lerpTimes have the same length.
    QVector<qint64> lerpValues;

    qint64 lerp(qint64 time) const;
    qint64 unlerp(qint64 time) const;
};

#endif // SNIPPET_H
