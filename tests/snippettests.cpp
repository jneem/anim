#include "snippettests.h"

#include "snippet.h"

SnippetTests::SnippetTests()
{
}

SnippetTests::~SnippetTests()
{
}

void SnippetTests::initTestCase()
{
}

void SnippetTests::cleanupTestCase()
{
}

void SnippetTests::test_repeated_lerps()
{
    QVector<ChangingPath*> empty;
    Snippet s(empty, 0, 100);
    QCOMPARE(s.lerp(0), 0);
    QCOMPARE(s.lerp(-10), 0);
    QCOMPARE(s.lerp(50), 50);
    QCOMPARE(s.lerp(100), 100);
    QCOMPARE(s.lerp(200), 100);

    s.addLerp(50, 25);
    QCOMPARE(s.lerp(50), 25);
    QCOMPARE(s.lerp(20), 10);
    QCOMPARE(s.unlerp(10), 20);
    QCOMPARE(s.lerp(100), 100);
    QCOMPARE(s.lerp(70), 55);
    QCOMPARE(s.unlerp(55), 70);

    // This block tests what happens when we lerp two different times to the same time.
    s.addLerp(55, 25); // 55 here refers to the "original" 70.
    // Now the whole interval from 50 to 70 should be lerped to 25.
    QCOMPARE(s.lerp(50), 25);
    QCOMPARE(s.lerp(60), 25);
    QCOMPARE(s.lerp(70), 25);
    QCOMPARE(s.lerp(80), 50);
    QCOMPARE(s.lerp(90), 75);
    QCOMPARE(s.unlerp(25), 50);
    QCOMPARE(s.unlerp(26), 70);

    // We can lerp more times to 25 if we want.
    s.addLerp(25, 25); // This should have no effect.
    s.addLerp(50, 25); // 50 is the original 80
    QCOMPARE(s.unlerp(25), 50);
    QCOMPARE(s.unlerp(26), 80);

    // This block tests what happens when we lerp the same time to two different times.
    Snippet t(empty, 0, 100);
    t.addLerp(50, 25);
    t.addLerp(25, 50); // 25 is the original 50.
    // Lerping an already-lerped time just modifies it. So we should be back to the identity lerp.
    QCOMPARE(t.unlerp(25), 25);
    QCOMPARE(t.unlerp(30), 30);
    QCOMPARE(t.unlerp(50), 50);
    QCOMPARE(t.unlerp(60), 60);
    QCOMPARE(t.lerp(50), 50);
    QCOMPARE(t.lerp(51), 51);
}

void SnippetTests::test_interlerping()
{
    QVector<ChangingPath*> empty;
    Snippet s(empty, 0, 100);

    s.addLerp(40, 20);
    // We lerp time 10 to *after* time 20, so we also need to modify the previous lerp.
    s.addLerp(10, 55);
    QCOMPARE(s.lerp(40), 60);
}
