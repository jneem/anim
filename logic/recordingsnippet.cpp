#include "recordingsnippet.h"

#include "changingpath.h"
#include "snippet.h"

#include <QDebug>
#include <QPointF>

RecordingSnippet::RecordingSnippet(qint64 start_time, QObject *parent) : QObject(parent)
{
    this->start_time = start_time;
    timer.start();
    qDebug() << "RecordingSnippet::startRecording() at time" << timer.elapsed();
}

void RecordingSnippet::startPath(const QPointF &pos)
{
    curPath = new ChangingPath(this);
    curPath->startAt(timer.elapsed(), pos);
}

void RecordingSnippet::lineTo(const QPointF &pos)
{
    if (curPath) {
        curPath->lineTo(timer.elapsed() + start_time, pos);
    }
}

void RecordingSnippet::finishPath(const QPointF &pos)
{
    if (curPath) {
        curPath->lineTo(timer.elapsed() + start_time, pos);
        paths.push_back(curPath);
        curPath = nullptr;
    }
}

Snippet *RecordingSnippet::finishRecording()
{
    qDebug() << "RecordingSnippet::finishRecording() at time" << timer.elapsed();
    Snippet *ret = new Snippet(paths, start_time, start_time + timer.elapsed(), parent());
    paths.clear();
    return ret;
}

const ChangingPath *RecordingSnippet::currentPath() const
{
    return curPath;
}
