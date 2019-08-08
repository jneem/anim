#ifndef DRAWINGAREA_H
#define DRAWINGAREA_H

#include <QElapsedTimer>
#include <QGraphicsView>
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
    void setAnimation(Animation *anim) { this->anim = anim; }
    Animation* animation() { return this->anim; }

signals:

public slots:
    void startRecording(qint64 time);
    void stopRecording();
    void update(qint64 prev_time, qint64 cur_time);

private:
    qint64 rec_start_time = 0;

    QGraphicsPathItem *curPathItem;
    RecordingSnippet *recSnippet;
    Animation *anim;
    QHash<const ChangingPath*, QGraphicsPathItem*> pathMap;
    QHash<QGraphicsPathItem*, const ChangingPath*> revPathMap;
};

#endif // DRAWINGAREA_H
