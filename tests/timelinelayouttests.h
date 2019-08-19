#ifndef TIMELINELAYOUTTESTS_H
#define TIMELINELAYOUTTESTS_H

#include <QtTest>

class TimelineLayoutTests : public QObject
{
    Q_OBJECT

public:
    TimelineLayoutTests();
    ~TimelineLayoutTests();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_case1();

};

#endif // TIMELINELAYOUTTESTS_H
