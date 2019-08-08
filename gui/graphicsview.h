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
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    qint64 currentTime() const { return curTime; }
    void setAnimation(Animation *anim) { this->anim = anim; }

signals:
    void startedPlaying();
    void stoppedPlaying();
    void playedTick(qint64);
    void startedRecording();
    void stoppedRecording();
    void changedLength(qint64);

public slots:
    void startRecording();
    void stopRecording();
    void startPlaying();
    void pausePlaying();
    void tick();
    void setTime(qint64 t);

private:
    qint64 curTime = 0;
    qint64 recStartTime = 0;

    QTimer *timer;
    QElapsedTimer elapsedTimer;
    QGraphicsPathItem *curPathItem;
    RecordingSnippet *recSnippet;
    Animation *anim;
    QHash<const ChangingPath*, QGraphicsPathItem*> pathMap;
    QHash<QGraphicsPathItem*, const ChangingPath*> revPathMap;
};

#endif // DRAWINGAREA_H
