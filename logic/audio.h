#ifndef AUDIO_H
#define AUDIO_H

#include <QAudioFormat>
#include <QBuffer>
#include <QObject>

class AudioSnippet;
class QIODevice;

class Audio : public QObject
{
    Q_OBJECT
public:
    explicit Audio(QObject *parent = nullptr);

    qint64 endTime() const;
    void writeAudio(QIODevice *, qint64 prev_time, qint64 cur_time);

signals:
    void snippetAdded(AudioSnippet *snip);
    void snippetRemoved(AudioSnippet *snip);

public slots:
    void addSnippet(AudioSnippet *snip);

private:
    QVector<AudioSnippet*> snippets;
    QVector<qint16> buf;
    QAudioFormat format;

    qint32 sample_count(qint64 time);
};

#endif // AUDIO_H
