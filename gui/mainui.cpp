#include "mainui.h"

#include <QDebug>
#include <QElapsedTimer>
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

#include "animation.h"
#include "graphicsview.h"
#include "timeline.h"

MainUI::MainUI(Animation *anim, QWidget *parent) : QWidget(parent)
{
    view = new GraphicsView(this);
    view->setAnimation(anim);

    timer = new QTimer;
    elapsed_timer = new QElapsedTimer;

    recButton = new QPushButton(QIcon(":/icons/media-record.png"), "");
    playButton = new QPushButton(QIcon(":/icons/media-playback-start.png"), "");
    QGroupBox *buttons = new QGroupBox(this);
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(recButton);
    buttonsLayout->addWidget(playButton);
    buttonsLayout->addStretch();
    buttons->setLayout(buttonsLayout);

    recButton->setShortcutEnabled(true);
    recButton->setShortcut(Qt::Key_R);
    playButton->setShortcutEnabled(true);
    playButton->setShortcut(Qt::Key_P);

    Timeline *timeline = new Timeline(this);

    connect(this, SIGNAL(startedPlaying()), timeline, SLOT(startPlaying()));
    connect(this, SIGNAL(stoppedPlaying()), timeline, SLOT(stopPlaying()));
    connect(this, SIGNAL(startedRecording()), timeline, SLOT(startRecording()));
    connect(this, SIGNAL(stoppedRecording()), timeline, SLOT(stopRecording()));
    connect(timeline, SIGNAL(timeWarped(qint64)), this, SLOT(setTime(qint64)));

    connect(anim, SIGNAL(snippetAdded(Snippet*, qint64)), timeline, SLOT(addSnippet(Snippet*, qint64)));
    connect(anim, SIGNAL(snippetRemoved(Snippet*)), timeline, SLOT(removeSnippet(Snippet*)));

    connect(timer, SIGNAL(timeout()), this, SLOT(tick()));
    connect(this, &MainUI::timeChanged, view, &GraphicsView::update);
    connect(this, &MainUI::timeChanged, timeline, [=](qint64, qint64 cur){ timeline->updateTime(cur); });

    idleButtonState();

    QVBoxLayout *topLayout = new QVBoxLayout;
    topLayout->addWidget(view);
    topLayout->addWidget(buttons);
    topLayout->addWidget(timeline);
    setLayout(topLayout);
}

void MainUI::setRecToStop()
{
    recButton->setEnabled(true);
    recButton->setIcon(QIcon(":/icons/media-playback-stop.png"));
    recButton->setToolTip("Stop recording [r]");
    disconnect(recButton, SIGNAL(clicked()), this, SLOT(startRecording()));
    connect(recButton, SIGNAL(clicked()), this, SLOT(stopRecording()));
}

void MainUI::setRecToRec()
{
    recButton->setEnabled(true);
    recButton->setIcon(QIcon(":/icons/media-record.png"));
    recButton->setToolTip("Start recording [r]");
    disconnect(recButton, SIGNAL(clicked()), this, SLOT(stopRecording()));
    connect(recButton, SIGNAL(clicked()), this, SLOT(startRecording()));
}

void MainUI::setPlayToStop()
{
    playButton->setEnabled(true);
    playButton->setIcon(QIcon(":/icons/media-playback-stop.png"));
    playButton->setToolTip("Stop playing [p]");
    disconnect(playButton, SIGNAL(clicked()), this, SLOT(startPlaying()));
    connect(playButton, SIGNAL(clicked()), this, SLOT(pausePlaying()));
}

void MainUI::setPlayToPlay()
{
    playButton->setEnabled(true);
    playButton->setIcon(QIcon(":/icons/media-playback-start.png"));
    playButton->setToolTip("Start playing [p]");
    disconnect(playButton, SIGNAL(clicked()), this, SLOT(pausePlaying()));
    connect(playButton, SIGNAL(clicked()), this, SLOT(startPlaying()));
}

void MainUI::playingButtonState()
{
    setPlayToStop();
    recButton->setEnabled(false);
}

void MainUI::recordingButtonState()
{
    setRecToStop();
    playButton->setEnabled(false);
}

void MainUI::idleButtonState()
{
    setRecToRec();
    setPlayToPlay();
}

void MainUI::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "key press";
}

void MainUI::keyReleaseEvent(QKeyEvent *event)
{
    qDebug() << "key release";
}

void MainUI::startRecording()
{
    qDebug() << "MainUI::startRecording";

    state = RECORDING;
    recordingButtonState();
    timer->start(16);
    elapsed_timer->start();

    view->startRecording(cur_time);
    emit startedRecording();
}

void MainUI::stopRecording()
{
    qDebug() << "MainUI::stopRecording";

    state = IDLE;
    idleButtonState();
    timer->stop();

    view->stopRecording();
    emit stoppedRecording();
}

void MainUI::startPlaying()
{
    // TODO: we should probably special-case based on the current time: if we're at the end, start playing from the beginning.
    // Otherwise, resume from the current time.
    emit timeChanged(cur_time, 0);
    cur_time = 0;
    state = PLAYING;
    playingButtonState();
    timer->start(16);
    elapsed_timer->start();

    emit startedPlaying();
}

void MainUI::pausePlaying()
{
    state = IDLE;
    idleButtonState();
    timer->stop();

    emit stoppedPlaying();
}

void MainUI::tick()
{
    qint64 prev_t = cur_time;
    qint64 t = prev_t + elapsed_timer->elapsed();
    if (state == SCANNING_BACKWARD) {
        t = prev_t - elapsed_timer->elapsed();
    }
    elapsed_timer->restart();
    cur_time = std::max(0LL, t);

    qDebug() << "time changed" << prev_t << cur_time;
    emit timeChanged(prev_t, cur_time);

    // If we are playing, we should stop at the end. If we are recording, we can
    // continue past it.
    if (state != RECORDING && (t > endTime() || t < 0)) {
        if (state == PLAYING) {
            pausePlaying();
        } else {
            state = IDLE;
            timer->stop();
        }
    }
}

void MainUI::setTime(qint64 t)
{
    emit timeChanged(cur_time, t);
    cur_time = t;
}
