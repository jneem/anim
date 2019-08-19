#ifndef ANIMATION_H
#define ANIMATION_H

#include <QObject>
#include <QHash>
#include <QMap>
#include <QSet>
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
    QVector<RenderedPath> updatedPaths(qint64 startTime, qint64 endTime);
    void addSnippet(Snippet*);
    qint64 endTime() const;
    void warpSnippet(Snippet *s, qint64 old_time, qint64 new_time);

signals:
    void snippetAdded(Snippet*);
    void snippetRemoved(Snippet*);
    void snippetChanged(Snippet*);

public slots:

private:
    qint64 relativeTime(Snippet*, qint64 t) const;

    QSet<Snippet*> snippets;

    // These snippets have been modified, so next time updatedPaths is called, they need
    // to be completely re-rendered.
    QSet<Snippet*> dirty_snippets;
};

#endif // ANIMATION_H
