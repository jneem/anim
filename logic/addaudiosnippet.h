#ifndef ADDAUDIOSNIPPET_H
#define ADDAUDIOSNIPPET_H

#include <QUndoCommand>

class Audio;
class AudioSnippet;

class AddAudioSnippet : public QUndoCommand
{
public:
    AddAudioSnippet(Audio *audio, AudioSnippet *snip);

    void redo() override;
    void undo() override;

private:
    Audio *audio;
    AudioSnippet *snip;
};

#endif // ADDAUDIOSNIPPET_H
