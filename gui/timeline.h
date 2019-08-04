#ifndef TIMELINE_H
#define TIMELINE_H

#include <QGraphicsScene>
#include <QGraphicsView>

class QGraphicsItem;
class Snippet;
class SnippetItem;

class Timeline : public QGraphicsView
{
    Q_OBJECT
public:
    explicit Timeline(QWidget *parent = nullptr);
    qreal unitsPerMs() const { return units_per_ms; }

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

signals:
    void timeWarped(qint64);
    void unitsPerSecondChanged(qreal);

public slots:
    void updateTime(qint64);
    void updateTimeBounds(qint64);
    void addSnippet(Snippet*, qint64 start_time);
    void removeSnippet(Snippet*);
    void startPlaying();
    void stopPlaying();
    void startRecording();
    void stopRecording();

private:
    void updateTimeFromPos(const QPoint &pos);

    QGraphicsScene *scene;
    QGraphicsItem *cursor;
    qint64 maxTime = 1;
    qreal units_per_ms = 4.0 / 1000.0;

    // Used to detect a click: if there is a mouse press followed by a
    // mouse release without moving in between, it counts as a click.
    QPoint mouse_press_location;

    // While playing or recording, we disable mouse interaction.
    bool playing_or_recording = false;
    bool dragging_cursor = false;

    QHash<Snippet*, SnippetItem*> snippet_to_item;
    QHash<Snippet*, qint64> snippet_to_start_time;
};

#endif // TIMELINE_H
