#ifndef ANIMATION_H
#define ANIMATION_H

#include <QObject>
#include <QVector>

class ChangingPath;
class QPainterPath;
class RenderedPath;
class Snippet;

class Animation: public QObject
{
    Q_OBJECT
public:
    explicit Animation(QObject *parent = nullptr);
    QVector<RenderedPath> updatedPaths(qint64 startTime, qint64 endTime) const;
    void addSnippet(Snippet*, qint64 startTime);
    qint64 endTime() const;

signals:
    void snippetAdded(Snippet*, qint64 start_time);
    void snippetRemoved(Snippet*);

public slots:

private:
    QVector<Snippet*> snippets;
    QVector<qint64> snippetStartTimes;
};

#endif // ANIMATION_H
