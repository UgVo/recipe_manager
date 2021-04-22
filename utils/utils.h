#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QList>
#include <QSet>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QSize>
#include <QWidget>

namespace recipe
{
    enum unit {g,L,dL,cL,mL,cuillere,pincee,none};
    extern QMap<int,QString> unitToString;

    enum modes{resume,display,edition};
    enum states{retracted,opened,transition,fixed};

    QString toCapitalised(QString str);

    QPropertyAnimation *slideAnimation(QWidget *parent, QPoint slide, QSize growth = QSize(), int time = 1000);
    QPropertyAnimation *growAnimation(QWidget *parent, QSize growth, int time = 1000);
    QPropertyAnimation *fadeAnimation(QWidget *parent, bool up, int time = 1000);
    QPropertyAnimation *inflateAnimation(QWidget *parent, QSize endSize, int time = 1000);
    QPropertyAnimation *deflateAnimation(QWidget *parent, int time = 1000);

    QPropertyAnimation *targetGeometryAnimation(QWidget* parent, QSize targetSize, QPoint targetPos, int time = 1000);
    QPropertyAnimation *targetPositionAnimation(QWidget* parent, QPoint targetPos, int time = 1000);
    QPropertyAnimation *targetSizeAnimation(QWidget* parent, QSize targetSize, int time = 1000);
}

#endif // UTILS_H
