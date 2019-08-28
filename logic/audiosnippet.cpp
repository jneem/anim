#include "audiosnippet.h"

#include <cmath>
#include <QAudioBuffer>
#include <QDebug>

AudioSnippet::AudioSnippet(QAudioBuffer *buffer, qint64 start_time, QObject *parent) : QObject(parent)
{
    qDebug() << "buffer duration" << buffer->duration() << ", bytes" << buffer->byteCount();
    buf = buffer;
    this->start_frame = static_cast<qint64>(round(start_time / 1000.0 * 44100.0));
}

AudioSnippet::~AudioSnippet()
{
    delete buf;
}

qint64
AudioSnippet::startTime() const
{
    return static_cast<qint64>(std::round(start_frame / 44100.0 * 1000.0));
}

qint64
AudioSnippet::startFrame() const
{
    return start_frame;
}

qint64
AudioSnippet::endTime() const
{
    // Note that audio buffer durations are in microseconds, but our return values is in milliseconds.
    return startTime() + (buf->duration() / 1000);
}

qint64
AudioSnippet::endFrame() const
{
    return startFrame() + buf->frameCount();
}
