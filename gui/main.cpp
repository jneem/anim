#include "animation.h"
#include "mainui.h"
#include <QApplication>

// TODO: add buttons for changing line thickness and color
// TODO: add an undo stack
// TODO: add an "instant" mode, recording without animation
// TODO: add an eraser
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Animation *anim = new Animation;
    MainUI *w = new MainUI(anim);
    w->show();

    return a.exec();
}
