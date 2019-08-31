#ifndef DRAWINGAREA_H
#define DRAWINGAREA_H

#include <QElapsedTimer>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QWidget>

class Animation;
class ChangingPath;
class QGraphicsScene;
class QPushButton;
class QTimer;
class RecordingSnippet;
class Snippet;

// By subclassing QGraphicsView, we can override the input handling.
class GraphicsView: public QGraphicsView
{
    Q_OBJECT
public:
    explicit GraphicsView(QWidget *parent = nullptr);

    void tabletEvent(QTabletEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    // Get rid of the default keyboard handlers.
    void keyPressEvent(QKeyEvent *ev) override { ev->ignore(); }
    void keyReleaseEvent(QKeyEvent *ev) override { ev->ignore(); }

    void setAnimation(Animation *anim) { this->anim = anim; }
    Animation* animation() { return this->anim; }

signals:

public slots:
    void startRecording(qint64 time);
    Snippet *stopRecording();
    void update(qint64 prev_time, qint64 cur_time);
    void addSnippet(Snippet *snip, qint64 cur_time);
    void removeSnippet(Snippet *snip);

private:
    qint64 rec_start_time = 0;

    QGraphicsPathItem *curPathItem;
    RecordingSnippet *recSnippet;
    Animation *anim;
    QHash<const ChangingPath*, QGraphicsPathItem*> pathMap;
    QHash<QGraphicsPathItem*, const ChangingPath*> revPathMap;
};

#endif // DRAWINGAREA_H
