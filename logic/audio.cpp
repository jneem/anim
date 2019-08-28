#include "audio.h"

#include "audiosnippet.h"

#include <QAudioBuffer>
#include <QDebug>

Audio::Audio(QObject *parent) : QObject(parent)
{
    // Our audio format needs to match the one used in MainUI::initializeAudio. Probably we should factor this out.
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::Endian::LittleEndian);
    format.setSampleRate(44100);
    format.setSampleSize(16);
    format.setChannelCount(1);
    format.setSampleType(QAudioFormat::SampleType::SignedInt);
}

void
Audio::addSnippet(AudioSnippet *snip)
{
    Q_ASSERT(snip->buffer()->format() == format);

    snip->setParent(this);
    snippets.push_back(snip);

    emit snippetAdded(snip);
}

qint64
Audio::endTime() const
{
    // TODO: this is inefficient
    qint64 ret = 0;
    for (AudioSnippet *s : snippets) {
        ret = std::max(ret, s->endTime());
    }
    return ret;
}

void
Audio::writeAudio(QIODevice *io, qint64 prev_time, qint64 cur_time)
{
    qint64 start_time = std::min(prev_time, cur_time);
    qint64 end_time = std::max(prev_time, cur_time);
    qint64 start_frame = sample_count(start_time);
    qint64 end_frame = sample_count(end_time);

    // Use our internal buffer for mixing the audio streams.
    buf.clear();
    buf.fill(0, static_cast<int>(end_frame - start_frame));

    // TODO: this is linear in the number of snippets
    for (auto snip: snippets) {
        if (snip->startFrame() > end_frame || snip->endFrame() < start_frame) {
            continue;
        }
        const qint16 *snip_buf = snip->buffer()->constData<qint16>();
        qint64 snip_buf_len = snip->buffer()->byteCount() / 2;
        qint64 start_offset = start_frame - snip->startFrame();
        qint64 end_offset = end_frame - snip->startFrame();
        end_offset = std::min(end_offset, snip_buf_len);

        // We'll copy offset i in our buffer to offset i + translation in mix_buffer.
        qint64 translation = -start_offset;
        start_offset = std::max(0LL, start_offset);

        if (end_offset + translation > buf.length()) {
            Q_ASSERT(false);
        }
        qDebug() << "playing audio from" << start_offset << "to" << end_offset;
        Q_ASSERT(end_offset <= snip_buf_len);
        for (auto i = start_offset; i < end_offset; i++) {
            buf[i + translation] += snip_buf[i];
        }
    }

    qDebug() << "writing" << buf.length() << "frames";
    io->write(reinterpret_cast<const char*>(buf.data()), buf.length() * 2);
}

qint32
Audio::sample_count(qint64 time)
{
    return format.framesForDuration(time * 1000);
}
