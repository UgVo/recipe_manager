#include "c_widget.h"

int c_widget::insideBorder = 4;
int c_widget::labelHeight = 20;
int c_widget::stepWidth = 606;
int c_widget::buttonHeight = 21;
int c_widget::borderSize = 9;
QMap<c_widget::modes,QString> c_widget::mapModeToString{
    {modes::display,"Display"},
    {modes::edition,"Edition"},
    {modes::minimal,"Minimal"},
    {modes::none,"None"},
    {modes::resume,"Resume"},
    {modes::setup,"Setup"}
};

c_widget::c_widget(QWidget *parent, c_widget *_parent) : QWidget(parent) {
    m_parent = _parent;
    mode = modes::none;
}

c_widget::~c_widget() {

}

QSize c_widget::getSize(c_widget::modes) const {
    return QSize();
}

int c_widget::getWidth(c_widget::modes) const {
    return 0;
}

QAbstractAnimation *c_widget::switchMode(c_widget::modes , bool, int) {
    return nullptr;
}

QAbstractAnimation *c_widget::switchMode(c_widget::modes, bool, int, QAnimationGroup *) {
    return nullptr;
}

int c_widget::getImageAreaWidth(c_widget::modes ) const {
    return width() - 2*borderSize;
}

int c_widget::getImageCount() const {
    return 1;
}

QAbstractAnimation *c_widget::runBehavior(bool animated, QAnimationGroup *group, QAnimationGroup *parentGroupAnimation) {
    if (animated) {
        if (parentGroupAnimation == nullptr) {
            if (m_parent != nullptr) {
                m_parent->handleChildrenAnimation(group);
            } else {
                group->start(QAbstractAnimation::DeleteWhenStopped);
            }
        } else {
            parentGroupAnimation->addAnimation(group);
        }
        return parentGroupAnimation;
    } else {
        delete group;
        return nullptr;
    }
}

void c_widget::handleChildrenAnimation(QAbstractAnimation *animation) {
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void c_widget::save() {

}

void c_widget::rollback() {

}

void c_widget::resizeEvent(QResizeEvent *) {
}

QAbstractAnimation *c_widget::fadeAnimation(QWidget *parent, bool up, int time, int delay) {
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    if ((up && parent->isHidden()) || (!up && !parent->isHidden())) {
        parent->show();
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(parent);
        parent->setGraphicsEffect(effect);
        QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity");
        animation->setDuration(time);
        animation->setKeyValueAt(0, up ? 0.0 : 1.0);
        animation->setKeyValueAt(double(delay)/double(time), up ? 0.0 : 1.0);
        animation->setKeyValueAt(1,up ? 1.0 : 0.0);
        animation->setEasingCurve(QEasingCurve::InOutQuad);

        QObject::connect(animation,&QPropertyAnimation::finished,[=] () {
            animation->disconnect();
            parent->setHidden(!up);
        });
        group->addAnimation(animation);
    }
    return group;
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
    QPropertyAnimation *animation = new QPropertyAnimation(parent,"pos");
    animation->setDuration(time);
    animation->setKeyValueAt(0,parent->pos());
    animation->setKeyValueAt(double(delay)/double(time), parent->pos());
    animation->setKeyValueAt(1,targetPos);
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
        parent->setFixedSize(targetSize);
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
    parent->setFixedSize(QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX));
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
    parent->setFixedSize(QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX));
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

QPropertyAnimation *c_widget::rotateAnimation(c_pixmapGraphics *parent, qreal angle, int time) {
    QPropertyAnimation *animation = new QPropertyAnimation(parent,"rotation");
    animation->setStartValue(parent->rotation());
    animation->setEndValue(angle);
    animation->setDuration(time);
    animation->setEasingCurve(QEasingCurve::InOutQuad);

    return animation;
}

QPropertyAnimation *c_widget::openMenuAnimation(QMenu *menu, int time) {
    QPropertyAnimation* animation = new QPropertyAnimation(menu, "size");
    animation->setDuration(time);
    animation->setStartValue(QSize(0, 0));
    animation->setEndValue(QSize(menu->width(), menu->height()));
    animation->setEasingCurve(QEasingCurve::InOutQuad);

    return animation;
}

QPropertyAnimation *c_widget::closeMenuAnimation(QMenu *menu, int time) {
    QPropertyAnimation* animation = new QPropertyAnimation(menu, "size");
    QSize menuSize = menu->size();
    menu->show();
    animation->setDuration(time);
    animation->setStartValue(menuSize);
    animation->setEndValue(QSize(0, 0));
    animation->setEasingCurve(QEasingCurve::InOutQuad);

    QObject::connect(animation,&QAbstractAnimation::finished,menu, [=] () {
       menu->setFixedSize(menuSize);
    });

    return animation;
}

int c_widget::getHorizontalAdvanceLabel(QLabel *label) const {
    int res = 0;
    res = QFontMetrics(label->font()).horizontalAdvance(label->text());
    return res;
}

c_widget::modes c_widget::getMode() const
{
    return mode;
}

void c_widget::setMode(c_widget::modes value)
{
    mode = value;
}
