#include <QtTest>
#include <QCoreApplication>
#include "timelinelayout.h"

// add necessary includes here

using std::make_pair;

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

TimelineLayoutTests::TimelineLayoutTests()
{
}

TimelineLayoutTests::~TimelineLayoutTests()
{
}

void TimelineLayoutTests::initTestCase()
{
}

void TimelineLayoutTests::cleanupTestCase()
{
}

void TimelineLayoutTests::test_case1()
{
    auto result = layoutTimeline<int>({   TimelineElement<int> { 1, 10, 0 },
                                          TimelineElement<int> { 5, 20, 1 },
                                          TimelineElement<int> { 11, 25, 2 },
                                          TimelineElement<int> { 6, 7, 3 },
    });

    auto expected = QMap<int, int>({make_pair(0, 0), make_pair(1, 1), make_pair(2, 0), make_pair(3, 2)});
    QCOMPARE(result, expected);
}

QTEST_MAIN(TimelineLayoutTests)

#include "tst_timelinelayouttests.moc"
