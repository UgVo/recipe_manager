#include "c_milestoneview.h"
#include "ui_c_milestoneview.h"

c_milestoneView::c_milestoneView(c_milestone *_milestone, c_widget *widget, QWidget *parent) :
    c_widget(parent,widget),
    ui(new Ui::c_milestoneView), milestone(_milestone) {
    ui->setupUi(this);

    if (milestone->getName().isEmpty()) {
        ui->milestoneButton->setText(QString("Jalon %1").arg(milestone->getRank()));
    } else {
        ui->milestoneButton->setText(milestone->getName());
    }
    QList<c_step *> steps = milestone->getStepsPtr();

    ui->milestoneButton->setFixedHeight(int(double(buttonHeight)*1.5));
    ui->milestoneButton->setFixedWidth(stepWidth);
    ui->milestoneButton->move(borderSize,borderSize);
    ui->milestoneButton->setStyleSheet("QPushButton#milestoneButton { "
                                       "text-align: left;"
                                       "font-size: 16px;"
                                       "padding-left: 10px;"
                                       "}");

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
        QObject::connect(stepList.last(),&c_stepView::swapRank,this,&c_milestoneView::slotSwapSteps);
        QObject::connect(stepList.last(),&c_stepView::toDelete,this,&c_milestoneView::slotDeleteSteps);
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

void c_milestoneView::slotSwapSteps(recipe::swap direction) {
    c_stepView *sender = static_cast<c_stepView *>(QObject::sender());
    if (milestone->swapSteps(sender->getStep(),direction)) {
        int index = stepList.indexOf(sender);
        if (direction == recipe::swapAbove) {
            stepList.swapItemsAt(index,index-1);
        } else if (direction == recipe::swapBelow) {
            stepList.swapItemsAt(index,index+1);
        }
    }
    switchMode(mode)->start(QAbstractAnimation::DeleteWhenStopped);
}

void c_milestoneView::slotDeleteSteps() {
    c_stepView *sender = static_cast<c_stepView *>(QObject::sender());
    if (milestone->removeStep(sender->getStep())) {
        stepList.removeOne(sender);
        sender->hide();
        sender->deleteLater();

        switchMode(mode)->start(QAbstractAnimation::DeleteWhenStopped);
    }
}
