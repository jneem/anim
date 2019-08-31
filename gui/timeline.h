#ifndef TIMELINE_H
#define TIMELINE_H

#include <QDebug>
#include <QKeyEvent>
#include <QGraphicsScene>
#include <QGraphicsView>

class AudioSnippet;
class AudioSnippetItem;
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

    // Get rid of the default handlers.
    void keyPressEvent(QKeyEvent *ev) override { ev->ignore(); }
    void keyReleaseEvent(QKeyEvent *ev) override { ev->ignore(); }

signals:
    void timeWarped(qint64);
    void unitsPerSecondChanged(qreal);
    void highlightedSnippet(Snippet*);

public slots:
    void updateTime(qint64);
    void addSnippet(Snippet*);
    void highlightSnippet(Snippet*);
    void removeSnippet(Snippet*);
    void updateSnippet(Snippet*);

    void addAudioSnippet(AudioSnippet*);
    void removeAudioSnippet(AudioSnippet*);

    void startPlaying();
    void stopPlaying();
    void startRecording();
    void stopRecording();
    void setMark(qint64);

private:
    void updateTimeFromPos(const QPoint &pos);
    void relayout();

    QGraphicsScene *scene = nullptr;
    QGraphicsItem *cursor = nullptr;
    QGraphicsItem *mark = nullptr;

    qint64 maxTime = 1;
    qreal units_per_ms = 4.0 / 1000.0;

    // Used to detect a click: if there is a mouse press followed by a
    // mouse release without moving in between, it counts as a click.
    QPoint mouse_press_location;

    // While playing or recording, we disable mouse interaction.
    bool playing_or_recording = false;
    bool dragging_cursor = false;

    // If there's a snippet that's currently highlighted, this is its item.
    SnippetItem *highlighted = nullptr;

    QHash<Snippet*, SnippetItem*> snippet_to_item;
    QHash<AudioSnippet*, AudioSnippetItem*> audio_snippet_to_item;
};

#endif // TIMELINE_H
