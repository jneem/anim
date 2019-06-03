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
    DrawingArea w;
    w.show();

    return a.exec();
}
