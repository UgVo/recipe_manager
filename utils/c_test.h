#ifndef C_TEST_H
#define C_TEST_H

#include <QtTest/QTest>
#include <QThread>
#include "utils/utils.h"
#include "utils/c_dbmanager.h"

class c_test : public QObject
{
    Q_OBJECT
public:
    c_test();

private slots:

    void types();
    void ingredient();
    void component();
    void step();
    void process();
    void notes();
    void milestone();
    void recipe();

private:
    c_dbManager* dbmanager;
    c_ioManager* iomanager;
};

#endif // C_TEST_H
