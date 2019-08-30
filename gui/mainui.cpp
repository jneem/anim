#include "mainui.h"

#include <QAction>
#include <QAudioBuffer>
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <QAudioOutput>
#include <QBuffer>
#include <QDebug>
#include <QElapsedTimer>
#include <QEvent>
#include <QGraphicsPathItem>
#include <QGraphicsView>
#include <QGridLayout>
#include <QGroupBox>
#include <QGuiApplication>
#include <QMenuBar>
#include <QPainterPath>
#include <QShortcut>
#include <QTabletEvent>
#include <QTimer>
#include <QPushButton>
#include <QVBoxLayout>

#include "animation.h"
#include "audio.h"
#include "audiosnippet.h"
#include "graphicsview.h"
#include "snippet.h"
#include "timeline.h"

MainUI::MainUI(Animation *anim, QWidget *parent) : QMainWindow(parent)
{
    initializeAudio();

    view = new GraphicsView(this);
    view->setAnimation(anim);

    timer = new QTimer;
    elapsed_timer = new QElapsedTimer;

    recButton = new QPushButton(QIcon(":/icons/media-record.png"), "");
    playButton = new QPushButton(QIcon(":/icons/media-playback-start.png"), "");
    audioButton = new QPushButton(QIcon(":/icons/audio-input-microphone.png"), "");
    QGroupBox *buttons = new QGroupBox(this);
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(recButton);
    buttonsLayout->addWidget(playButton);
    buttonsLayout->addWidget(audioButton);
    buttonsLayout->addStretch();
    buttons->setLayout(buttonsLayout);

    QVBoxLayout *topLayout = new QVBoxLayout;
    QWidget *topWidget = new QWidget;
    timeline = new Timeline(this);
    topLayout->addWidget(view);
    topLayout->addWidget(buttons);
    topLayout->addWidget(timeline);
    topWidget->setLayout(topLayout);
    setCentralWidget(topWidget);

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

    connect(audio, &Audio::snippetAdded, timeline, &Timeline::addAudioSnippet);

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

    initializeActions();
    idleButtonState();
}

void MainUI::initializeAudio()
{
    QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    audio_format.setCodec("audio/pcm");
    audio_format.setByteOrder(QAudioFormat::Endian::LittleEndian);
    audio_format.setSampleRate(44100);
    audio_format.setSampleSize(16);
    audio_format.setChannelCount(1);
    audio_format.setSampleType(QAudioFormat::SampleType::SignedInt);

    if (!info.isFormatSupported(audio_format)) {
        qFatal("unsupported audio format");
    }

    audio_input = new QAudioInput(audio_format, this);
    audio_output = new QAudioOutput(audio_format, this);
    audio = new Audio(this);
    audio_recording_buffer = new QBuffer(this);
    audio_recording_buffer->open(QIODevice::ReadWrite);
}

void
MainUI::initializeActions()
{
    auto openAct = new QAction("&New", this);
    openAct->setShortcuts(QKeySequence::Open);

    auto saveAct = new QAction("&Save", this);
    saveAct->setShortcuts(QKeySequence::Save);

    auto saveAsAct = new QAction("&Save as", this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);

    auto quitAct = new QAction("&Quit", this);
    quitAct->setShortcuts(QKeySequence::Quit);

    auto fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAct);

    auto undoAct = new QAction("&Undo", this);
    undoAct->setShortcuts(QKeySequence::Undo);

    auto redoAct = new QAction("&Redo", this);
    redoAct->setShortcuts(QKeySequence::Redo);

    auto editMenu = menuBar()->addMenu("&Edit");
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);
}

UIState
MainUI::uiState() const
{
    return state;
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

void MainUI::setAudioToRec()
{
    audioButton->setEnabled(true);
    audioButton->setIcon(QIcon(":/icons/audio-input-microphone.png"));
    audioButton->setToolTip("Start recording audio [a]");
    disconnect(audioButton, &QPushButton::clicked, this, &MainUI::stopRecordingAudio);
    connect(audioButton, &QPushButton::clicked, this, &MainUI::startRecordingAudio);
}

void MainUI::setAudioToStop()
{
    audioButton->setEnabled(true);
    audioButton->setIcon(QIcon(":/icons/media-playback-stop.png"));
    audioButton->setToolTip("Stop recording audio [a]");
    disconnect(audioButton, &QPushButton::clicked, this, &MainUI::startRecordingAudio);
    connect(audioButton, &QPushButton::clicked, this, &MainUI::stopRecordingAudio);
}

void MainUI::playingButtonState()
{
    setPlayToStop();
    recButton->setEnabled(false);
    audioButton->setEnabled(false);
}

void MainUI::recordingButtonState()
{
    setRecToStop();
    playButton->setEnabled(false);
    audioButton->setEnabled(false);
}

void MainUI::recordingAudioButtonState()
{
    playButton->setEnabled(false);
    recButton->setEnabled(false);
    setAudioToStop();
}

void MainUI::idleButtonState()
{
    setRecToRec();
    setPlayToPlay();
    setAudioToRec();
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

    audio_output_device = audio_output->start();

    emit startedPlaying();
}

void MainUI::pausePlaying()
{
    state = IDLE;
    idleButtonState();
    timer->stop();

    audio_output->stop();
    audio_output_device = nullptr;

    emit stoppedPlaying();
}

void MainUI::startRecordingAudio()
{
    state = RECORDING_AUDIO;
    recordingAudioButtonState();
    timer->start(16);
    elapsed_timer->start();

    audio_recording_buffer->reset();
    audio_recording_buffer->buffer().clear();
    audio_input->start(audio_recording_buffer);
    audio_start_time = cur_time;

    emit startedRecordingAudio();
}

void MainUI::stopRecordingAudio()
{
    state = IDLE;
    idleButtonState();
    timer->stop();
    audio_input->stop();

    QAudioBuffer *buf = new QAudioBuffer(audio_recording_buffer->buffer(), audio_format);
    audio->addSnippet(new AudioSnippet(buf, audio_start_time, this));

    emit stoppedRecordingAudio();
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

    if (audio_output_device && (state == SCANNING_FORWARD || state == PLAYING)) {
        audio->writeAudio(audio_output_device, prev_t, cur_time);
    }

    //qDebug() << "time changed" << prev_t << cur_time;
    emit timeChanged(prev_t, cur_time);

    // If we are playing, we should stop at the end. If we are recording, we can
    // continue past it.
    if (state != RECORDING && state != RECORDING_AUDIO && (t > endTime() || t < 0)) {
        if (state == PLAYING) {
            pausePlaying();
        } else {            state = IDLE;
            timer->stop();
        }
    }
}

void MainUI::setTime(qint64 t)
{
    emit timeChanged(cur_time, t);
    cur_time = t;
}

qint64 MainUI::endTime() const
{
    return std::max(audio->endTime(), view->animation()->endTime());

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
