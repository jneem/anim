#include "addsnippet.h"

#include "animation.h"

AddSnippet::AddSnippet(Animation *anim, Snippet *snip)
{
    this->anim = anim;
    this->snip = snip;
    setText("Add snippet");
}

void
AddSnippet::redo()
{
    anim->addSnippet(snip);
}

void
AddSnippet::undo()
{
    anim->removeSnippet(snip);
}
