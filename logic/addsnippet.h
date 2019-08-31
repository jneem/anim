#ifndef ADDSNIPPET_H
#define ADDSNIPPET_H

#include <QUndoCommand>

class Animation;
class Snippet;

class AddSnippet : public QUndoCommand
{
public:
    AddSnippet(Animation *anim, Snippet *snip);

    void undo() override;
    void redo() override;

private:
    Animation *anim;
    Snippet *snip;
};

#endif // ADDSNIPPET_H
