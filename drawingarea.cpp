#include "drawingarea.h"

#include "animation.h"
#include "changingpath.h"
#include "recordingsnippet.h"
#include "snippet.h"
#include "timeline.h"

#include <QDebug>
#include <QEvent>
#include <QGraphicsPathItem>
#include <QGraphicsView>
#include <QGridLayout>
#include <QGroupBox>
#include <QPainterPath>
#include <QTabletEvent>
#include <QTimer>
#include <QPushButton>
#include <QVBoxLayout>

GraphicsView::GraphicsView(QWidget *parent) : QGraphicsView(parent)
{
    QGraphicsScene *scene = new QGraphicsScene(this);
    setScene(scene);
    setSceneRect(0, 0, 640, 480);
    setAnimation(anim);
    setRenderHint(QPainter::Antialiasing);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QGraphicsItem *border = new QGraphicsRectItem(0, 0, 640, 480);
    scene->addItem(border);

    curPathItem = nullptr;
    recSnippet = nullptr;
    anim = nullptr;

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(tick()));
}

void GraphicsView::tabletEvent(QTabletEvent *event)
{
    event->accept();

    // TODO: tablet events support high-resolution positions, but I don't see how to map them to the scene.
    if (recSnippet) {
        QPointF pos = mapToScene(event->pos());
        if (event->type() == QEvent::TabletPress) {
            recSnippet->startPath(pos);
            curPathItem = new QGraphicsPathItem();
            scene()->addItem(curPathItem);

            pathMap.insert(recSnippet->currentPath(), curPathItem);
            revPathMap.insert(curPathItem, recSnippet->currentPath());
        } else if (event->type() == QEvent::TabletMove) {
            recSnippet->lineTo(pos);
            if (curPathItem) {
                curPathItem->setPath(recSnippet->currentPath()->toPainterPath());
            }
        } else if (event->type() == QEvent::TabletRelease) {
            if (curPathItem) {
                curPathItem->setPath(recSnippet->currentPath()->toPainterPath());
                curPathItem = nullptr;
            }
            recSnippet->finishPath(pos);
        }
    }
}

void GraphicsView::resizeEvent(QResizeEvent *)
{
    // Essentially, we want to display a 640x480 grid, but make it a little larger to allow for the border.
    QRectF sceneRect(-2, -2, 642, 482);
    setSceneRect(sceneRect);
    centerOn(sceneRect.center());
    setTransform(QTransform());

    qreal scalex = width() / sceneRect.width();
    qreal scaley = height() / sceneRect.height();
    qreal s = std::min(scalex, scaley);
    scale(s, s);
}

void GraphicsView::startRecording()
{
    recSnippet = new RecordingSnippet(this);
    recStartTime = curTime;

    // TODO: also play back existing things while recording.
    emit startedRecording();
}

void GraphicsView::stopRecording()
{
    if (recSnippet) {
        Snippet *snippet = recSnippet->finishRecording();
        delete recSnippet;
        recSnippet = nullptr;
        anim->addSnippet(snippet, recStartTime);

        // This is not really accurate. Plus, we should be updating the length as we record.
        emit changedLength(recStartTime + snippet->endTime());
        emit stoppedRecording();
    }
}

void GraphicsView::startPlaying()
{
    // TODO: we should probably special-case based on the current time: if we're at the end, start playing from the beginning.
    // Otherwise, resume from the current time.
    curTime = 0;
    for (auto i = pathMap.constBegin(); i != pathMap.constEnd(); i++) {
        // set all the paths to empty
        i.value()->setPath(QPainterPath());
    }
    timer->start(16);
    elapsedTimer.start();

    emit startedPlaying();
}

void GraphicsView::pausePlaying()
{
    timer->stop();

    emit stoppedPlaying();
}

void GraphicsView::tick()
{
    qint64 prev_t = curTime;
    qint64 t = prev_t + elapsedTimer.elapsed();
    elapsedTimer.restart();
    curTime = t;

    auto pathsToUpdate = anim->updatedPaths(prev_t, t);
    for (auto p: pathsToUpdate) {
        QGraphicsPathItem *item = pathMap.value(p.path);
        if (item) {
            item->setPath(p.rendered);
        } else {
            qDebug() << "failed to find item for path" << p.path;
        }
    }

    if (t > anim->endTime()) {
        pausePlaying();
        emit stoppedPlaying();
    } else {
        emit playedTick(curTime);
    }
}

void GraphicsView::setTime(qint64 t)
{
    curTime = t;
    // First, erase all the paths. Then re-render them.
    for (auto i = pathMap.constBegin(); i != pathMap.constEnd(); i++) {
        i.value()->setPath(QPainterPath());
    }

    auto pathsToUpdate = anim->updatedPaths(0, t);
    for (auto p: pathsToUpdate) {
        QGraphicsPathItem *item = pathMap.value(p.path);
        if (item) {
            item->setPath(p.rendered);
        } else {
            qDebug() << "failed to find item for path" << p.path;
        }
    }
}

DrawingArea::DrawingArea(Animation *anim, QWidget *parent) : QWidget(parent)
{
    view = new GraphicsView(this);
    view->setAnimation(anim);

    recButton = new QPushButton(QIcon::fromTheme("media-record"), "Record");
    playButton = new QPushButton(QIcon::fromTheme("media-playback-start"), "Play");
    stopButton = new QPushButton(QIcon::fromTheme("media-playback-stop"), "Stop");
    QGroupBox *buttons = new QGroupBox(this);
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(recButton);
    buttonsLayout->addWidget(playButton);
    buttonsLayout->addWidget(stopButton);
    buttons->setLayout(buttonsLayout);

    Timeline *timeline = new Timeline(this);

    connect(recButton, SIGNAL(clicked()), view, SLOT(startRecording()));
    connect(stopButton, SIGNAL(clicked()), view, SLOT(stopRecording()));
    connect(playButton, SIGNAL(clicked()), view, SLOT(startPlaying()));

    connect(view, SIGNAL(startedPlaying()), this, SLOT(playingButtonState()));
    connect(view, SIGNAL(stoppedPlaying()), this, SLOT(idleButtonState()));
    connect(view, SIGNAL(startedRecording()), this, SLOT(recordingButtonState()));
    connect(view, SIGNAL(stoppedRecording()), this, SLOT(idleButtonState()));

    connect(view, SIGNAL(playedTick(qint64)), timeline, SLOT(updateTime(qint64)));
    connect(view, SIGNAL(changedLength(qint64)), timeline, SLOT(updateTimeBounds(qint64)));
    connect(timeline, SIGNAL(timeWarped(qint64)), view, SLOT(setTime(qint64)));

    idleButtonState();

    QVBoxLayout *topLayout = new QVBoxLayout;
    topLayout->addWidget(view);
    topLayout->addWidget(buttons);
    topLayout->addWidget(timeline);
    setLayout(topLayout);
}

void DrawingArea::playingButtonState()
{
    recButton->setEnabled(false);
    playButton->setEnabled(false);
    stopButton->setEnabled(true);
    disconnect(stopButton, SIGNAL(clicked()));
    connect(stopButton, SIGNAL(clicked()), view, SLOT(pausePlaying()));
}

void DrawingArea::recordingButtonState()
{
    recButton->setEnabled(false);
    playButton->setEnabled(false);
    stopButton->setEnabled(true);
    disconnect(stopButton, SIGNAL(clicked()));
    connect(stopButton, SIGNAL(clicked()), view, SLOT(stopRecording()));
}

void DrawingArea::idleButtonState()
{
    recButton->setEnabled(true);
    playButton->setEnabled(true);
    stopButton->setEnabled(false);
}
