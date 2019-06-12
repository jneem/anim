#ifndef TIMELINE_H
#define TIMELINE_H

#include <QGraphicsScene>
#include <QGraphicsView>

class QGraphicsLineItem;

class Timeline : public QGraphicsView
{
    Q_OBJECT
public:
    explicit Timeline(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void timeWarped(qint64);

public slots:
    void updateTime(qint64);
    void updateTimeBounds(qint64);

private:
    QGraphicsScene *scene;
    QGraphicsLineItem *cursor;
    qint64 maxTime = 1;
};

#endif // TIMELINE_H
