#include "animation.h"
#include "drawingarea.h"
#include "mainwindow.h"
#include <QApplication>

// TODO: add a timer slider for replay
// TODO: add buttons for changing line thickness and color
// TODO: add an undo stack
// TODO: add an eraser
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Animation *anim = new Animation(&a);
    DrawingArea w(anim);
    w.show();

    return a.exec();
}
