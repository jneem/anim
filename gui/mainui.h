#ifndef MAINUI_H
#define MAINUI_H

#include <QWidget>

class Animation;
class GraphicsView;
class QPushButton;

class MainUI : public QWidget
{
    Q_OBJECT
public:
    explicit MainUI(Animation *anim, QWidget *parent = nullptr);

signals:

public slots:
    void playingButtonState();
    void recordingButtonState();
    void idleButtonState();

private:
    void setRecToRec();
    void setRecToStop();
    void setPlayToPlay();
    void setPlayToStop();

    GraphicsView *view;

    QPushButton *recButton;
    QPushButton *playButton;
};


#endif // MAINUI_H
