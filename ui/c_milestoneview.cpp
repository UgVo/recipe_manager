#include "c_milestoneview.h"
#include "ui_c_milestoneview.h"

c_milestoneView::c_milestoneView(c_milestone *_milestone, QWidget *parent) :
    c_widget(parent),
    ui(new Ui::c_milestoneView), milestone(_milestone) {
    ui->setupUi(this);

    QList<c_step *> steps = milestone->getStepsPtr();

    ui->milestoneButton->setFixedHeight(buttonHeight);
    ui->milestoneButton->setFixedWidth(stepWidth);
    ui->milestoneButton->move(borderSize,borderSize);

    QObject::connect(ui->milestoneButton,&QPushButton::clicked, [=] () {
        switch (mode) {
        case modes::display:
            switchMode(modes::minimal,true,500)->start(QAbstractAnimation::DeleteWhenStopped);
            break;
        case modes::minimal:
            switchMode(modes::display,true,500)->start(QAbstractAnimation::DeleteWhenStopped);
            break;
        default:
            switchMode(modes::minimal,true,500)->start(QAbstractAnimation::DeleteWhenStopped);
            break;
        }
    });

    this->setFixedWidth(stepWidth + borderSize*2);

    for (int i = 0; i < steps.size(); ++i) {
        stepList.push_back(new c_stepView(steps[i],this));
        QObject::connect(stepList.last(),&c_stepView::animationRequired,this,&c_milestoneView::slotHandleResizeStep);
    }

    mode = modes::minimal;
    switchMode(mode,false);
}

c_milestoneView::~c_milestoneView() {
    delete ui;
}

QAbstractAnimation *c_milestoneView::switchMode(c_widget::modes target, bool animated, int time, QAbstractAnimation *childAnims) {
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    if (childAnims != nullptr)
        group->addAnimation(childAnims);
    switch (target) {
    case modes::minimal:
    case modes::display: {
        if (animated) {
            group->addAnimation(targetSizeAnimation(this,getSize(target),time));
        } else {
            this->setFixedSize(getSize(target));
        }

        QPoint pos(borderSize,borderSize + ui->milestoneButton->height() + insideBorder);
        for (int i = 0; i < stepList.size(); ++i) {
            if (animated) {
                group->addAnimation(targetPositionAnimation(stepList[i],pos,time));
            } else {
                stepList[i]->move(pos);
            }
            pos += QPoint(0,stepList[i]->getSize().height() + insideBorder);
        }
        break;
    }
    default:
        break;
    }

    mode = target;

    return group;
}

QSize c_milestoneView::getSize(modes target) const {
    QSize res;
    switch (target) {
    case modes::minimal:
        res.setHeight(borderSize + ui->milestoneButton->height() + insideBorder);
        res.setWidth(borderSize + getStepDisplaySize().width() + borderSize);
        break;
    case modes::display: {
        QSize stepDisplaySize = getStepDisplaySize();
        res.setHeight(borderSize + ui->milestoneButton->height() + insideBorder + stepDisplaySize.height() + borderSize);
        res.setWidth(borderSize + stepDisplaySize.width() + borderSize);
        break;
    }
    default:
        break;
    }
    return res;
}

QSize c_milestoneView::getStepDisplaySize() const {
    int heightSteps = 0;
    for (int i = 0; i < stepList.size(); ++i) {
        heightSteps += stepList[i]->getSize().height();
    }
    heightSteps += (stepList.size()-1)*insideBorder;
    return QSize(stepWidth,heightSteps);
}

void c_milestoneView::slotHandleResizeStep(QAbstractAnimation *animation) {
    c_stepView *sender = static_cast<c_stepView *>(QObject::sender());
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    int indexStep = stepList.indexOf(sender);
    QPoint pos = stepList[indexStep]->pos() + QPoint(0,insideBorder + stepList[indexStep]->getSize().height());
    for (int i = indexStep+1; i < stepList.size(); ++i) {
        group->addAnimation(targetPositionAnimation(stepList[i],pos,animation->duration()));
        pos += QPoint(0,stepList[i]->getSize().height() + insideBorder);
    }
    group->addAnimation(targetSizeAnimation(this,getSize(mode),animation->duration()));
    group->addAnimation(animation);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}
