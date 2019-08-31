#include "addaudiosnippet.h"

#include "audio.h"

AddAudioSnippet::AddAudioSnippet(Audio *audio, AudioSnippet *snip)
{
    this->audio = audio;
    this->snip = snip;
}

void
AddAudioSnippet::redo()
{
    audio->addSnippet(snip);
}

void
AddAudioSnippet::undo()
{
    audio->removeSnippet(snip);
}
