#ifndef CHANGINGPATH_H
#define CHANGINGPATH_H

#include <QElapsedTimer>
#include <QObject>
#include <QPainterPath>
#include <QPointF>
#include <QVector>

class ChangingPath : public QObject
{
    Q_OBJECT
public:
    explicit ChangingPath(QObject *parent = nullptr);

    void simplify();
    void lineTo(qint64 time, const QPointF &pos);
    void startAt(qint64 time, const QPointF &pos);
    qint64 startTime() const;
    qint64 endTime() const;
    QPainterPath toPainterPath(qint64 time) const;
    QPainterPath toPainterPath() const;
signals:

public slots:

private:
    // TODO: add color and thickness
    QVector<QPointF> points;
    QVector<qint64> times;
    int last_simplified_idx = 0;
};

struct RenderedPath {
    const ChangingPath *path;
    QPainterPath rendered;
};

#endif // CHANGINGPATH_H
