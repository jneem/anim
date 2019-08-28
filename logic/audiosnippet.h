#ifndef AUDIOSNIPPET_H
#define AUDIOSNIPPET_H

#include <QObject>

class QAudioBuffer;

class AudioSnippet : public QObject
{
    Q_OBJECT
public:
    AudioSnippet(QAudioBuffer *buffer, qint64 start_time, QObject *parent = nullptr);
    virtual ~AudioSnippet();

    qint64 startTime() const;
    qint64 endTime() const;
    qint64 startFrame() const;
    qint64 endFrame() const;
    QAudioBuffer *buffer() { return this->buf; }

signals:

public slots:

private:
    QAudioBuffer *buf;

    // Internally, we always work with frames (i.e. 1/44100 of a second) instead of times in milliseconds.
    qint64 start_frame;
};

#endif // AUDIOSNIPPET_H
