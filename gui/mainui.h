#ifndef MAINUI_H
#define MAINUI_H

#include <QAudioFormat>
#include <QWidget>

#include "animation.h"
#include "graphicsview.h"

class Audio;
class Timeline;
class QBuffer;
class QElapsedTimer;
class QPushButton;
class QTimer;
class QAudioOutput;
class QAudioInput;


enum UIState {
    // When the right/left arrows are held down, we scan the animation.
    SCANNING_FORWARD,
    SCANNING_BACKWARD,
    PLAYING,
    RECORDING,
    RECORDING_AUDIO,
    IDLE,
};

class MainUI : public QWidget
{
    Q_OBJECT
public:
    explicit MainUI(Animation *anim, QWidget *parent = nullptr);
    qint64 currentTime() const { return cur_time; }
    qint64 endTime() const;

signals:
    void startedPlaying();
    void stoppedPlaying();
    void startedRecording();
    void stoppedRecording();
    void startedRecordingAudio();
    void stoppedRecordingAudio();
    void timeChanged(qint64 prev, qint64 cur);
    void markSet(qint64);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

public slots:
    void playingButtonState();
    void recordingButtonState();
    void recordingAudioButtonState();
    void idleButtonState();

    void startPlaying();
    void pausePlaying();
    void startRecording();
    void stopRecording();
    void startRecordingAudio();
    void stopRecordingAudio();
    void tick();
    void setTime(qint64 t);

    //void addSnippet(Snippet*, qint64);
    void removeSnippet(Snippet*);
    void focusSnippet(Snippet*);

    void setMark();
    void warpToMark();

    void snapBackToKeyFrame();
    void snapForwardToKeyFrame();

private:
    void setRecToRec();
    void setRecToStop();
    void setPlayToPlay();
    void setPlayToStop();
    void setAudioToRec();
    void setAudioToStop();
    void stopScanning();
    qint64 timeFactor();

    void initializeAudio();

    GraphicsView *view;
    Timeline *timeline;

    QPushButton *recButton;
    QPushButton *playButton;
    QPushButton *audioButton;

    UIState state = IDLE;
    QTimer *timer;
    QElapsedTimer *elapsed_timer;
    qint64 cur_time = 0;
    qint64 mark_time = 0;

    Audio *audio;
    QAudioInput *audio_input = nullptr;
    QAudioOutput *audio_output = nullptr;
    QIODevice *audio_output_device = nullptr;
    QBuffer *audio_recording_buffer = nullptr;
    qint64 audio_start_time;
    QAudioFormat audio_format;

    Snippet *focused_snippet = nullptr;
};


#endif // MAINUI_H
