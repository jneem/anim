#ifndef SNIPPETTESTS_H
#define SNIPPETTESTS_H

#include <QtTest>

class SnippetTests: public QObject
{
    Q_OBJECT

public:
    SnippetTests();
    ~SnippetTests();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_repeated_lerps();
    void test_interlerping();
};

#endif // SNIPPETTESTS_H
