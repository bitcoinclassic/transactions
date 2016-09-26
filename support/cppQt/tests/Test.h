#ifndef TEST_H
#define TEST_H

#include <QObject>
#include <QtTest/QtTest>

class Test : public QObject {
    Q_OBJECT
public:
    Test() {}

private slots:
    void testBasis();
    void testTypes();
};

#endif

