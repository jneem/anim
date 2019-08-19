#ifndef MAINUI_H
#define MAINUI_H

#include <QWidget>

#include "animation.h"
#include "graphicsview.h"

class Timeline;
class QElapsedTimer;
class QPushButton;
class QTimer;


enum UIState {
    // When the right/left arrows are held down, we scan the animation.
    SCANNING_FORWARD,
    SCANNING_BACKWARD,
    PLAYING,
    RECORDING,
    IDLE,
};

class MainUI : public QWidget
{
    Q_OBJECT
public:
    explicit MainUI(Animation *anim, QWidget *parent = nullptr);
    qint64 currentTime() const { return cur_time; }
    qint64 endTime() const { return view->animation()->endTime(); }

signals:
    void startedPlaying();
    void stoppedPlaying();
    void startedRecording();
    void stoppedRecording();
    void timeChanged(qint64 prev, qint64 cur);
    void markSet(qint64);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

public slots:
    void playingButtonState();
    void recordingButtonState();
    void idleButtonState();

    void startPlaying();
    void pausePlaying();
    void startRecording();
    void stopRecording();
    void tick();
    void setTime(qint64 t);

    //void addSnippet(Snippet*, qint64);
    void removeSnippet(Snippet*);
    void focusSnippet(Snippet*);

    void setMark();
    void warpToMark();

private:
    void setRecToRec();
    void setRecToStop();
    void setPlayToPlay();
    void setPlayToStop();
    void stopScanning();

    GraphicsView *view;
    Timeline *timeline;

    QPushButton *recButton;
    QPushButton *playButton;

    UIState state = IDLE;
    QTimer *timer;
    QElapsedTimer *elapsed_timer;
    qint64 cur_time = 0;
    qint64 mark_time = 0;

    Snippet *focused_snippet = nullptr;
};


#endif // MAINUI_H
