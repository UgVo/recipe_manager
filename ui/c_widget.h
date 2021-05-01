#ifndef C_WIDGET_H
#define C_WIDGET_H

#include <QSize>
#include <QWidget>
#include <QAnimationGroup>
#include <QPropertyAnimation>
#include <utils/utils.h>

class c_widget : public QWidget
{
public:
    c_widget(QWidget *parent = nullptr);

    virtual ~c_widget();

    virtual QSize getSize(int mode);
    virtual QAnimationGroup *switchMode(int mode = recipe::modes::resume, bool animated = true,int time = 1000);

    virtual void save();
    virtual void rollback();

protected:
    QPropertyAnimation *fadeAnimation(QWidget *parent, bool up, int time = 1000, int delay = 0);
    QPropertyAnimation *targetGeometryAnimation(QWidget* parent, QSize targetSize, QPoint targetPos, int time = 1000);
    QPropertyAnimation *targetPositionAnimation(QWidget* parent, QPoint targetPos, int time = 1000);
    QPropertyAnimation *targetSizeAnimation(QWidget* parent, QSize targetSize, int time = 1000);
    QAnimationGroup *slideAndDeployAnimation(QWidget* parent, QPoint targetPos, int time = 1000,
                                             std::function<void()> lambda = nullptr, int mode = recipe::modes::resume);


    int mode;
    int state;

    enum modes{resume,display,edition};
    enum states{retracted,opened,transition,fixed};
};

#endif // C_WIDGET_H
