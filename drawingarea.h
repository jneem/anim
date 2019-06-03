#ifndef DRAWINGAREA_H
#define DRAWINGAREA_H

#include <QGraphicsView>
#include <QWidget>

class ChangingPath;
class QGraphicsScene;
class QPushButton;

// By subclassing QGraphicsView, we can override the input handling.
class GraphicsView: public QGraphicsView
{
    Q_OBJECT
    public:
        explicit GraphicsView(QWidget *parent = nullptr);

        void tabletEvent(QTabletEvent *event) override;

    private:
        QGraphicsPathItem *curPathItem;
        ChangingPath *path;
        QPointF lastControl;

        void finishPath(const QPointF &pos);
};

class DrawingArea : public QWidget
{
    Q_OBJECT
public:
    explicit DrawingArea(QWidget *parent = nullptr);

signals:

public slots:

private:
    QGraphicsScene *scene;
    GraphicsView *view;
    QPushButton *clearButton;
};

#endif // DRAWINGAREA_H
