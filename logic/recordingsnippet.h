#ifndef RECORDINGSNIPPET_H
#define RECORDINGSNIPPET_H

#include <QElapsedTimer>
#include <QObject>
#include <QVector>

class ChangingPath;
class QPointF;
class Snippet;

class RecordingSnippet : public QObject
{
    Q_OBJECT
public:
    explicit RecordingSnippet(QObject *parent = nullptr);

    void startPath(const QPointF &pos);
    void lineTo(const QPointF &pos);
    void finishPath(const QPointF &pos);
    const ChangingPath *currentPath() const;
    Snippet *finishRecording();

signals:

public slots:

private:
    QVector<ChangingPath*> paths;
    QElapsedTimer timer;
    ChangingPath *curPath;
};

#endif // RECORDINGSNIPPET_H
