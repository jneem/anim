#ifndef MAINUI_H
#define MAINUI_H

#include <QWidget>

#include "animation.h"
#include "graphicsview.h"

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

private:
    void setRecToRec();
    void setRecToStop();
    void setPlayToPlay();
    void setPlayToStop();

    GraphicsView *view;

    QPushButton *recButton;
    QPushButton *playButton;

    UIState state = IDLE;
    QTimer *timer;
    QElapsedTimer *elapsed_timer;
    qint64 cur_time = 0;
};


#endif // MAINUI_H
