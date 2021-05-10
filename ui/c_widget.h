#ifndef C_WIDGET_H
#define C_WIDGET_H

#include <QDebug>
#include <QSize>
#include <QWidget>
#include <QAnimationGroup>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QSequentialAnimationGroup>
#include <utils/utils.h>

class c_widget : public QWidget
{
public:
    enum modes{resume,display,edition};
    enum states{retracted,opened,transition,fixed};

    c_widget(QWidget *parent = nullptr);

    virtual ~c_widget();

    virtual QSize getSize(modes target) const;
    virtual int getWidth(modes target) const;
    virtual QAbstractAnimation *switchMode(modes target = modes::resume, bool animated = true,int time = 1000);
    virtual QAbstractAnimation *switchMode(modes target = modes::resume, bool animated = true,int time = 1000, QAbstractAnimation *childAnims = nullptr);

    virtual void save();
    virtual void rollback();

    modes getMode() const;
    void setMode(modes value);

protected:
    QPropertyAnimation *fadeAnimation(QWidget *parent, bool up, int time = 1000, int delay = 0);
    QPropertyAnimation *targetGeometryAnimation(QWidget* parent, QSize targetSize, QPoint targetPos, int time = 1000);
    QPropertyAnimation *targetPositionAnimation(QWidget* parent, QPoint targetPos, int time = 1000, int delay = 0);
    QPropertyAnimation *targetSizeAnimation(QWidget* parent, QSize targetSize, int time = 1000);
    QAnimationGroup *slideAndDeployAnimation(QWidget* parent, QPoint targetPos, int time = 1000,
                                             std::function<void()> lambda = nullptr, modes mode = modes::resume);
    QPropertyAnimation *deflateAnimation(QWidget *parent, int time);
    QPropertyAnimation *inflateAnimation(QWidget *parent, QSize endSize, int time);

    modes mode;
    states state;

    static int insideBorder;
    static int labelHeight;
};

#endif // C_WIDGET_H
