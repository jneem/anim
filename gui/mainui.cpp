#include "mainui.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QEvent>
#include <QGraphicsPathItem>
#include <QGraphicsView>
#include <QGridLayout>
#include <QGroupBox>
#include <QGuiApplication>
#include <QPainterPath>
#include <QShortcut>
#include <QTabletEvent>
#include <QTimer>
#include <QPushButton>
#include <QVBoxLayout>

#include "animation.h"
#include "graphicsview.h"
#include "snippet.h"
#include "timeline.h"

// TODO: make scanning speed variable (e.g., maybe hold shift to make it fast)

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

    QVBoxLayout *topLayout = new QVBoxLayout;
    timeline = new Timeline(this);
    topLayout->addWidget(view);
    topLayout->addWidget(buttons);
    topLayout->addWidget(timeline);
    setLayout(topLayout);

    recButton->setShortcutEnabled(true);
    recButton->setShortcut(Qt::Key_R);
    playButton->setShortcutEnabled(true);
    playButton->setShortcut(Qt::Key_P);

    connect(this, SIGNAL(startedPlaying()), timeline, SLOT(startPlaying()));
    connect(this, SIGNAL(stoppedPlaying()), timeline, SLOT(stopPlaying()));
    connect(this, SIGNAL(startedRecording()), timeline, SLOT(startRecording()));
    connect(this, SIGNAL(stoppedRecording()), timeline, SLOT(stopRecording()));
    connect(this, &MainUI::markSet, timeline, &Timeline::setMark);
    connect(timeline, &Timeline::timeWarped, this, &MainUI::setTime);
    connect(timeline, &Timeline::highlightedSnippet, this, &MainUI::focusSnippet);

    connect(anim, &Animation::snippetAdded, timeline, &Timeline::addSnippet);
    connect(anim, &Animation::snippetRemoved, timeline, &Timeline::removeSnippet);
    connect(anim, &Animation::snippetChanged, timeline, &Timeline::updateSnippet);
    //connect(anim, &Animation::snippetAdded, this, &MainUI::addSnippet);
    connect(anim, &Animation::snippetRemoved, this, &MainUI::removeSnippet);

    connect(timer, SIGNAL(timeout()), this, SLOT(tick()));
    connect(this, &MainUI::timeChanged, view, &GraphicsView::update);
    connect(this, &MainUI::timeChanged, timeline, [=](qint64, qint64 cur){ timeline->updateTime(cur); });

    auto mark_shortcut = new QShortcut(QKeySequence(Qt::Key_M), this);
    auto warp_shortcut = new QShortcut(QKeySequence(Qt::Key_W), this);
    connect(mark_shortcut, &QShortcut::activated, this, &MainUI::setMark);
    connect(warp_shortcut, &QShortcut::activated, this, &MainUI::warpToMark);

    auto snap_forward_shortcut = new QShortcut(QKeySequence(Qt::Key_Greater), this);
    auto snap_back_shortcut = new QShortcut(QKeySequence(Qt::Key_Less), this);
    connect(snap_forward_shortcut, &QShortcut::activated, this, &MainUI::snapForwardToKeyFrame);
    connect(snap_back_shortcut, &QShortcut::activated, this, &MainUI::snapBackToKeyFrame);

    idleButtonState();
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
    //qDebug() << "key press";
    if (event->isAutoRepeat()) {
        return;
    }
    if (state == IDLE) {
        if (event->key() == Qt::Key_Left) {
            event->accept();
            state = SCANNING_BACKWARD;
            timer->start(16);
            elapsed_timer->restart();
        } else if (event->key() == Qt::Key_Right) {
            event->accept();
            state = SCANNING_FORWARD;
            timer->start(16);
            elapsed_timer->restart();
        }
    } else if (state == SCANNING_FORWARD || state == SCANNING_BACKWARD) {
        stopScanning();
    }
}

void MainUI::stopScanning()
{
    state = IDLE;
    timer->stop();
}

void MainUI::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) {
        return;
    }
    //qDebug() << "key release";

    if (state == SCANNING_FORWARD || state == SCANNING_BACKWARD) {
        // Even if the released key wasn't the arrow key, stop scanning.
        event->accept();
        stopScanning();
    }
}

void MainUI::startRecording()
{
    //qDebug() << "MainUI::startRecording";

    state = RECORDING;
    recordingButtonState();
    timer->start(16);
    elapsed_timer->start();

    view->startRecording(cur_time);
    emit startedRecording();
}

void MainUI::stopRecording()
{
    //qDebug() << "MainUI::stopRecording";

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
    qint64 t = prev_t + elapsed_timer->elapsed() * timeFactor();
    if (state == SCANNING_BACKWARD) {
        t = prev_t - elapsed_timer->elapsed() * timeFactor();
    }
    elapsed_timer->restart();
    cur_time = std::max(0LL, t);

    //qDebug() << "time changed" << prev_t << cur_time;
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

void MainUI::removeSnippet(Snippet *snip)
{
    if (focused_snippet == snip) {
        focused_snippet = nullptr;
    }
}

void MainUI::focusSnippet(Snippet *snip)
{
    focused_snippet = snip;
    timeline->highlightSnippet(snip);
}

void MainUI::setMark()
{
    //qDebug() << "set mark" << cur_time;
    mark_time = cur_time;
    emit markSet(mark_time);
}

void MainUI::warpToMark()
{
    if (focused_snippet) {
        //qDebug() << "warp to mark" << cur_time;
        view->animation()->warpSnippet(focused_snippet, cur_time, mark_time);
        setTime(mark_time);
    }
}

void MainUI::snapBackToKeyFrame()
{
    if (focused_snippet) {
        auto times = focused_snippet->keyTimes();
        auto iter = std::lower_bound(times.constBegin(), times.constEnd(), cur_time);
        if (iter != times.constBegin()) {
            setTime(*(iter - 1));
        }
    }
}

void MainUI::snapForwardToKeyFrame()
{
    if (focused_snippet) {
        auto times = focused_snippet->keyTimes();
        auto iter = std::upper_bound(times.constBegin(), times.constEnd(), cur_time);
        if (iter != times.constEnd()) {
            setTime(*iter);
        }
    }
}

qint64 MainUI::timeFactor() {
    if ((state == SCANNING_FORWARD || state == SCANNING_BACKWARD)
            && ((QGuiApplication::keyboardModifiers() & Qt::ShiftModifier) != 0)) {
        return 4;
    } else {
        return 1;
    }
}
