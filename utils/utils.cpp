#include "utils.h"
#include "c_component.h"
#include "c_dbmanager.h"
#include "c_recipe.h"
#include "c_ingredient.h"

namespace recipe {
    QMap<int,QString> unitToString {
        {unit::g,"g"},
        {unit::L,"L"},
        {unit::dL,"dL"},
        {unit::cL,"cL"},
        {unit::mL,"mL"},
        {unit::cuillere,QObject::tr("cuillère")},
        {unit::pincee,QObject::tr("pincée")},
        {unit::none,""}
    };

    QPropertyAnimation *slideAnimation(QWidget *parent, QPoint slide, QSize growth, int time) {
        QRect rect;
        QSize size;
        QPropertyAnimation *animation = new QPropertyAnimation(parent,"geometry");
        animation->setDuration(time);
        rect = parent->rect();
        size = parent->size();
        rect.setTopLeft(parent->pos());
        rect.setSize(size);
        animation->setStartValue(rect);
        rect.setTopLeft(parent->pos() + slide);
        rect.setSize(size + growth);
        animation->setEndValue(rect);
        animation->setEasingCurve(QEasingCurve::InOutQuart);

        return animation;
    }

    QPropertyAnimation *growAnimation(QWidget *parent, QSize growth, int time) {
        QPropertyAnimation *animation = new QPropertyAnimation(parent, "size");
        animation->setDuration(time);
        animation->setStartValue(parent->size());
        animation->setEndValue(parent->size() + growth);

        animation->setEasingCurve(QEasingCurve::InOutQuart);

        return animation;
    }

    QPropertyAnimation *fadeAnimation(QWidget *parent, bool up) {
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(parent);
        parent->setGraphicsEffect(effect);
        QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity");
        animation->setDuration(1000);
        animation->setStartValue(up ? 0.0 : 1.0);
        animation->setEndValue(up ? 1.0 : 0.0);
        animation->setEasingCurve(QEasingCurve::InOutQuart);

        return animation;
    }

    QPropertyAnimation *inflateAnimation(QWidget *parent, QSize endSize, int time)     {
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
        animation->setEasingCurve(QEasingCurve::InOutQuart);

        return animation;
    }

    QPropertyAnimation *deflateAnimation(QWidget *parent, int time)     {
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
        animation->setEasingCurve(QEasingCurve::InOutQuart);

        return animation;
    }

}
