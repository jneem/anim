#ifndef CHANGINGPATH_H
#define CHANGINGPATH_H

#include <QElapsedTimer>
#include <QObject>
#include <QPointF>
#include <QVector>

class QPainterPath;

class ChangingPath : public QObject
{
    Q_OBJECT
public:
    explicit ChangingPath(QObject *parent = nullptr);

    void simplify();
    void lineTo(const QPointF &pos);
    void startAt(const QPointF &pos);
    QPainterPath toPainterPath() const;
signals:

public slots:

private:
    QVector<QPointF> points;
    QVector<qreal> times;
    int last_simplified_idx = 0;
    QElapsedTimer timer;
};

#endif // CHANGINGPATH_H
