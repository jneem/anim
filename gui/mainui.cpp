#include "mainui.h"

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

#include "animation.h"
#include "graphicsview.h"
#include "timeline.h"

MainUI::MainUI(Animation *anim, QWidget *parent) : QWidget(parent)
{
    view = new GraphicsView(this);
    view->setAnimation(anim);

    recButton = new QPushButton(QIcon(":/icons/media-record.png"), "");
    playButton = new QPushButton(QIcon(":/icons/media-playback-start.png"), "");
    QGroupBox *buttons = new QGroupBox(this);
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(recButton);
    buttonsLayout->addWidget(playButton);
    buttonsLayout->addStretch();
    buttons->setLayout(buttonsLayout);

    Timeline *timeline = new Timeline(this);

    connect(view, SIGNAL(startedPlaying()), this, SLOT(playingButtonState()));
    connect(view, SIGNAL(stoppedPlaying()), this, SLOT(idleButtonState()));
    connect(view, SIGNAL(startedRecording()), this, SLOT(recordingButtonState()));
    connect(view, SIGNAL(stoppedRecording()), this, SLOT(idleButtonState()));

    connect(view, SIGNAL(playedTick(qint64)), timeline, SLOT(updateTime(qint64)));
    connect(view, SIGNAL(changedLength(qint64)), timeline, SLOT(updateTimeBounds(qint64)));
    connect(view, SIGNAL(startedPlaying()), timeline, SLOT(startPlaying()));
    connect(view, SIGNAL(stoppedPlaying()), timeline, SLOT(stopPlaying()));
    connect(view, SIGNAL(startedRecording()), timeline, SLOT(startRecording()));
    connect(view, SIGNAL(stoppedRecording()), timeline, SLOT(stopRecording()));
    connect(timeline, SIGNAL(timeWarped(qint64)), view, SLOT(setTime(qint64)));

    connect(anim, SIGNAL(snippetAdded(Snippet*, qint64)), timeline, SLOT(addSnippet(Snippet*, qint64)));
    connect(anim, SIGNAL(snippetRemoved(Snippet*)), timeline, SLOT(removeSnippet(Snippet*)));

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
    disconnect(recButton, SIGNAL(clicked()), view, SLOT(startRecording()));
    connect(recButton, SIGNAL(clicked()), view, SLOT(stopRecording()));
}

void MainUI::setRecToRec()
{
    recButton->setEnabled(true);
    recButton->setIcon(QIcon(":/icons/media-record.png"));
    disconnect(recButton, SIGNAL(clicked()), view, SLOT(stopRecording()));
    connect(recButton, SIGNAL(clicked()), view, SLOT(startRecording()));
}

void MainUI::setPlayToStop()
{
    playButton->setEnabled(true);
    playButton->setIcon(QIcon(":/icons/media-playback-stop.png"));
    disconnect(playButton, SIGNAL(clicked()), view, SLOT(startPlaying()));
    connect(playButton, SIGNAL(clicked()), view, SLOT(pausePlaying()));
}

void MainUI::setPlayToPlay()
{
    playButton->setEnabled(true);
    playButton->setIcon(QIcon(":/icons/media-playback-start.png"));
    disconnect(playButton, SIGNAL(clicked()), view, SLOT(pausePlaying()));
    connect(playButton, SIGNAL(clicked()), view, SLOT(startPlaying()));
}

void MainUI::playingButtonState()
{
    qDebug() << "playingButtonState";
    setPlayToStop();
    recButton->setEnabled(false);
}

void MainUI::recordingButtonState()
{
    qDebug() << "recordingButtonState";
    setRecToStop();
    playButton->setEnabled(false);
}

void MainUI::idleButtonState()
{
    qDebug() << "idleButtonState";
    setRecToRec();
    setPlayToPlay();
}
