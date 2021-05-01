#include "c_widget.h"

c_widget::c_widget(QWidget *parent) : QWidget(parent) {

}

c_widget::~c_widget() {

}

QSize c_widget::getSize(int) {
    return QSize();
}

QAnimationGroup *c_widget::switchMode(int, bool, int) {
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
        animation->setEasingCurve(QEasingCurve::InOutQuart);

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
    animation->setEasingCurve(QEasingCurve::InOutQuart);

    QObject::connect(animation,&QPropertyAnimation::finished,[=] () {
        animation->disconnect();
        parent->setFixedSize(targetSize);
    });

    return animation;
}

QPropertyAnimation *c_widget::targetPositionAnimation(QWidget *parent, QPoint targetPos, int time) {
    QRect rect;
    QPropertyAnimation *animation = new QPropertyAnimation(parent,"geometry");
    animation->setDuration(time);
    rect = parent->rect();
    rect.setTopLeft(parent->pos());
    rect.setSize(parent->size());
    animation->setStartValue(rect);
    rect.setTopLeft(targetPos);
    rect.setSize(parent->size());
    animation->setEndValue(rect);
    animation->setEasingCurve(QEasingCurve::InOutQuart);

    return animation;
}

QPropertyAnimation *c_widget::targetSizeAnimation(QWidget *parent, QSize targetSize, int time) {
    parent->setFixedSize(QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX));
    QPropertyAnimation *animation = new QPropertyAnimation(parent, "size");
    animation->setDuration(time);
    animation->setStartValue(parent->size());
    animation->setEndValue(targetSize);

    animation->setEasingCurve(QEasingCurve::InOutQuart);

    QObject::connect(animation,&QPropertyAnimation::finished,[=] () {
        animation->disconnect();
        parent->setFixedSize(targetSize);
    });

    return animation;
}

QAnimationGroup *c_widget::slideAndDeployAnimation(QWidget *parent, QPoint targetPos, int time, std::function<void()> lambda, int mode) {
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
    animation->setEasingCurve(QEasingCurve::InOutQuart);
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
    animation->setEasingCurve(QEasingCurve::InOutQuart);
    res->addAnimation(animation);
    res->addAnimation(animation);

    return res;
}
