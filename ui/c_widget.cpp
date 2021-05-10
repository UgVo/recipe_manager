#include "c_widget.h"

int c_widget::insideBorder = 4;
int c_widget::labelHeight = 20;

c_widget::c_widget(QWidget *parent) : QWidget(parent) {

}

c_widget::~c_widget() {

}

QSize c_widget::getSize(modes) const {
    return QSize();
}

int c_widget::getWidth(modes) const {
    return 0;
}

QAbstractAnimation *c_widget::switchMode(modes , bool, int) {
    return nullptr;
}

QAbstractAnimation *c_widget::switchMode(modes, bool, int, QAbstractAnimation*) {
    return nullptr;
}

void c_widget::save() {

}

void c_widget::rollback() {

}

QPropertyAnimation *c_widget::fadeAnimation(QWidget *parent, bool up, int time, int delay) {
    QPropertyAnimation *animation = nullptr;
    if ((up && parent->isHidden()) || (!up && !parent->isHidden())) {
        parent->show();
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(parent);
        parent->setGraphicsEffect(effect);
        animation = new QPropertyAnimation(effect, "opacity");
        animation->setDuration(time);
        animation->setKeyValueAt(0, up ? 0.0 : 1.0);
        animation->setKeyValueAt(double(delay)/double(time), up ? 0.0 : 1.0);
        animation->setKeyValueAt(1,up ? 1.0 : 0.0);
        animation->setEasingCurve(QEasingCurve::InOutQuad);

        QObject::connect(animation,&QPropertyAnimation::finished,[=] () {
            animation->disconnect();
            parent->setHidden(!up);
        });
    }
    return animation;
}

QPropertyAnimation *c_widget::targetGeometryAnimation(QWidget *parent, QSize targetSize, QPoint targetPos, int time) {
    parent->setFixedSize(QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX));
    QRect rect;
    QPropertyAnimation *animation = new QPropertyAnimation(parent,"geometry");
    animation->setDuration(time);
    rect = parent->rect();
    rect.setTopLeft(parent->pos());
    rect.setSize(parent->size());
    animation->setStartValue(rect);
    rect.setTopLeft(targetPos);
    rect.setSize(targetSize);
    animation->setEndValue(rect);
    animation->setEasingCurve(QEasingCurve::InOutQuad);

    QObject::connect(animation,&QPropertyAnimation::finished,[=] () {
        animation->disconnect();
        //parent->setFixedSize(targetSize);
    });

    return animation;
}

QPropertyAnimation *c_widget::targetPositionAnimation(QWidget *parent, QPoint targetPos, int time, int delay) {
    QRect rect;
    QPropertyAnimation *animation = new QPropertyAnimation(parent,"geometry");
    animation->setDuration(time);
    rect = parent->rect();
    rect.setTopLeft(parent->pos());
    rect.setSize(parent->size());
    animation->setKeyValueAt(0, rect);
    animation->setKeyValueAt(double(delay)/double(time), rect);
    rect.setTopLeft(targetPos);
    rect.setSize(parent->size());
    animation->setKeyValueAt(1,rect);
    animation->setEasingCurve(QEasingCurve::InOutQuad);

    return animation;
}

QPropertyAnimation *c_widget::targetSizeAnimation(QWidget *parent, QSize targetSize, int time) {
    parent->setFixedSize(QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX));
    QPropertyAnimation *animation = new QPropertyAnimation(parent, "size");
    animation->setDuration(time);
    animation->setStartValue(parent->size());
    animation->setEndValue(targetSize);

    animation->setEasingCurve(QEasingCurve::InOutQuad);

    QObject::connect(animation,&QPropertyAnimation::finished,[=] () {
        animation->disconnect();
        //parent->setFixedSize(targetSize);
    });

    return animation;
}

QAnimationGroup *c_widget::slideAndDeployAnimation(QWidget *parent, QPoint targetPos, int time, std::function<void()> lambda, modes mode) {
    QSequentialAnimationGroup *res = new QSequentialAnimationGroup();
    QRect rect;
    QPropertyAnimation *animation = new QPropertyAnimation(parent,"geometry");
    animation->setDuration(time/4);
    rect = parent->rect();
    rect.setTopLeft(parent->pos());
    rect.setSize(parent->size());
    animation->setStartValue(rect);
    rect.setTopLeft(QPoint(parent->pos().x(),-static_cast<c_widget *>(parent)->getSize(mode).height()));
    rect.setSize(parent->size());
    animation->setEndValue(rect);
    animation->setEasingCurve(QEasingCurve::InOutQuad);
    res->addAnimation(animation);

    if (lambda != nullptr) {
        QObject::connect(animation,&QPropertyAnimation::finished,lambda);
    }

    animation = new QPropertyAnimation(parent,"geometry");
    animation->setDuration(time - time/4);
    rect = parent->rect();
    rect.setTopLeft(QPoint(0,-static_cast<c_widget *>(parent)->getSize(mode).height()));
    rect.setSize(parent->size());
    animation->setStartValue(rect);
    rect.setTopLeft(targetPos);
    rect.setSize(parent->size());
    animation->setEndValue(rect);
    animation->setEasingCurve(QEasingCurve::InOutQuad);
    res->addAnimation(animation);
    res->addAnimation(animation);

    return res;
}

QPropertyAnimation *c_widget::deflateAnimation(QWidget *parent, int time) {
    QRect rect;
    QSize size;
    QPoint initPoint = parent->pos();
    QPropertyAnimation *animation = new QPropertyAnimation(parent,"geometry");
    animation->setDuration(time);
    rect = parent->rect();
    size = parent->size();
    rect.setTopLeft(initPoint);
    rect.setSize(parent->size());
    animation->setStartValue(rect);
    rect.setTopLeft(initPoint + QPoint(parent->width()/2,parent->height()/2));
    rect.setSize(QSize(0,0));
    animation->setEndValue(rect);
    animation->setEasingCurve(QEasingCurve::InOutQuad);

    return animation;
}

QPropertyAnimation *c_widget::inflateAnimation(QWidget *parent, QSize endSize, int time) {
    QRect rect;
    QSize size;
    QPoint initPoint = parent->pos();
    QPropertyAnimation *animation = new QPropertyAnimation(parent,"geometry");
    animation->setDuration(time);
    rect = parent->rect();
    size = parent->size();
    rect.setTopLeft(initPoint + QPoint(parent->width()/2,parent->height()/2));
    rect.setSize(QSize(0,0));
    animation->setStartValue(rect);
    rect.setTopLeft(initPoint);
    rect.setSize(endSize);
    animation->setEndValue(rect);
    animation->setEasingCurve(QEasingCurve::InOutQuad);

    return animation;
}

c_widget::modes c_widget::getMode() const
{
    return mode;
}

void c_widget::setMode(modes value)
{
    mode = value;
}
