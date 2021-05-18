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

    ui->newStepButton->setFixedHeight(buttonHeight);
    ui->newStepButton->setFixedWidth(stepWidth);
    QObject::connect(ui->newStepButton,&QPushButton::clicked,this,&c_milestoneView::slotAddStep);

    ui->milestoneButton->setFixedHeight(int(double(buttonHeight)*1.5));
    ui->milestoneButton->setFixedWidth(stepWidth + 2*(borderSize-insideBorder));
    ui->milestoneButton->move(insideBorder,insideBorder);
    ui->milestoneButton->setStyleSheet(QString("QPushButton#milestoneButton { "
                                       "text-align: left;"
                                       "font-size: 16px;"
                                       "padding-left: %1px;"
                                       "}").arg(borderSize));
    ui->milestoneButton->style()->unpolish(ui->milestoneButton);
    ui->milestoneButton->style()->polish(ui->milestoneButton);
    ui->milestoneButton->update();

    ui->milestoneNameEdit->setText(ui->milestoneButton->text());
    ui->milestoneNameEdit->setFixedHeight(ui->milestoneButton->height()-4);
    ui->milestoneNameEdit->setFont(ui->milestoneButton->font());
    QObject::connect(ui->milestoneNameEdit,&QLineEdit::textChanged,this,&c_milestoneView::slotUpdateCurrentCharCount);

    arrow = new QLabel("test",ui->milestoneButton);
    arrowPixmapUp = QPixmap(":/images/collapse-arrow.png");
    arrowPixmapDown = QPixmap(":/images/open-arrow.png");

    arrow->setFixedSize(ui->milestoneButton->height(),ui->milestoneButton->height());
    arrow->setPixmap(arrowPixmapUp);
    arrow->setScaledContents(true);

    QObject::connect(ui->milestoneButton,&QPushButton::clicked, [=] () {
        switch (mode) {
        case modes::display:
            switchMode(modes::minimal,true,500)->start(QAbstractAnimation::DeleteWhenStopped);
            break;
        case modes::minimal:
            switchMode(defaultMode,true,500)->start(QAbstractAnimation::DeleteWhenStopped);
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
        QObject::connect(stepList.last(),&c_stepView::saved,this,&c_milestoneView::slotUpdateProcesses);
    }

    processResume = nullptr;

    slotUpdateProcesses();

    processResume = new c_processView(getProcessesPtr(),this,ui->milestoneButton);
    delete processResume->switchMode(modes::resume,false);

    this->setStyleSheet("outline : 0;");

    mode = modes::minimal;
    defaultMode = modes::display;
    delete c_milestoneView::switchMode(mode,false);
}

c_milestoneView::~c_milestoneView() {
    delete ui;
}

QAbstractAnimation *c_milestoneView::switchMode(c_widget::modes target, bool animated, int time, QAbstractAnimation *childAnims) {
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    if (childAnims != nullptr)
        group->addAnimation(childAnims);
    switch (target) {
    case modes::minimal: {
        arrow->move(ui->milestoneButton->width() - arrow->width()-insideBorder,0);
        arrow->setPixmap(arrowPixmapUp);
        if (animated) {
            group->addAnimation(targetSizeAnimation(this,getSize(target),time));
        } else {
            this->setFixedSize(getSize(target));
        }

        QPoint pos(borderSize,insideBorder + ui->milestoneButton->height() + insideBorder);
        for (int i = 0; i < stepList.size(); ++i) {
            if (animated) {
                group->addAnimation(targetPositionAnimation(stepList[i],pos,time));
            } else {
                stepList[i]->move(pos);
            }
            pos += QPoint(0,stepList[i]->getSize().height() + insideBorder);
        }

        pos = QPoint( ui->milestoneButton->width() - arrow->width() - borderSize - processResume->getSize(modes::resume).width(),insideBorder);
        if (animated) {
            group->addAnimation(targetPositionAnimation(processResume,pos,time));
        } else {
            processResume->move(pos);
        }

        pos = QPoint(borderSize,getSize(modes::edition).height());
        if (animated) {
            group->addAnimation(targetPositionAnimation(ui->newStepButton,pos,time));
        } else {
            ui->newStepButton->move(pos);
        }

        ui->milestoneButton->setText(ui->milestoneNameEdit->text());
        ui->milestoneNameEdit->hide();
        ui->charCount->hide();
        break;
    }
    case modes::display: {
        arrow->move(ui->milestoneButton->width() - arrow->width()-insideBorder,0);
        arrow->setPixmap(arrowPixmapDown);
        if (animated) {
            group->addAnimation(targetSizeAnimation(this,getSize(target),time));
        } else {
            this->setFixedSize(getSize(target));
        }

        QPoint pos(borderSize,insideBorder + ui->milestoneButton->height() + insideBorder);
        for (int i = 0; i < stepList.size(); ++i) {
            if (animated) {
                group->addAnimation(targetPositionAnimation(stepList[i],pos,time));
            } else {
                stepList[i]->move(pos);
            }
            pos += QPoint(0,stepList[i]->getSize().height() + insideBorder);
        }

        pos = QPoint( ui->milestoneButton->width() - arrow->width() - borderSize - processResume->getSize(modes::resume).width(),ui->milestoneButton->height());
        if (animated) {
            group->addAnimation(targetPositionAnimation(processResume,pos,time));
        } else {
            processResume->move(pos);
        }

        pos = QPoint(borderSize,getSize(modes::edition).height());
        if (animated) {
            group->addAnimation(targetPositionAnimation(ui->newStepButton,pos,time));
        } else {
            ui->newStepButton->move(pos);
        }

        ui->milestoneNameEdit->hide();
        ui->charCount->hide();
        break;
    }
    case modes::edition: {
        arrow->move(ui->milestoneButton->width() - arrow->width()-insideBorder,0);
        arrow->setPixmap(arrowPixmapDown);
        if (animated) {
            group->addAnimation(targetSizeAnimation(this,getSize(target),time));
        } else {
            this->setFixedSize(getSize(target));
        }

        QPoint pos(borderSize,insideBorder + ui->milestoneButton->height() + insideBorder);
        for (int i = 0; i < stepList.size(); ++i) {
            if (animated) {
                group->addAnimation(targetPositionAnimation(stepList[i],pos,time));
            } else {
                stepList[i]->move(pos);
            }
            pos += QPoint(0,stepList[i]->getSize().height() + insideBorder);
        }
        pos = QPoint( ui->milestoneButton->width() - arrow->width() - borderSize - processResume->getSize(modes::resume).width(),ui->milestoneButton->height());
        if (animated) {
            group->addAnimation(targetPositionAnimation(processResume,pos,time));
        } else {
            processResume->move(pos);
        }

        ui->milestoneNameEdit->show();
        ui->milestoneNameEdit->setText(ui->milestoneButton->text());
        pos = QPoint(insideBorder + borderSize - 2,insideBorder + (ui->milestoneButton->height() - ui->milestoneNameEdit->height())/2);
        QSize targetSize(int(double(ui->milestoneButton->width()*0.6)),ui->milestoneNameEdit->height());
        ui->milestoneNameEdit->move(pos);
        ui->milestoneNameEdit->setFixedWidth(QFontMetrics(ui->milestoneButton->font()).horizontalAdvance(ui->milestoneButton->text()) + 8);
        if (animated) {
            group->addAnimation(targetSizeAnimation(ui->milestoneNameEdit,targetSize,time));
        } else {
            ui->milestoneNameEdit->setFixedSize(targetSize);
        }
        QObject::connect(group,&QAbstractAnimation::finished,group,[=] () {
            slotUpdateCurrentCharCount();
            ui->charCount->move(QPoint(targetSize.width() + borderSize + insideBorder - 2 - ui->charCount->width(),insideBorder + (ui->milestoneButton->height() - ui->charCount->height())/2));
            ui->charCount->show();
        });

        pos = QPoint(borderSize,getSize(modes::edition).height()-insideBorder - ui->newStepButton->height());
        if (animated) {
            group->addAnimation(targetPositionAnimation(ui->newStepButton,pos,time));
        } else {
            ui->newStepButton->move(pos);
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
        res.setHeight(insideBorder + ui->milestoneButton->height() + insideBorder);
        res.setWidth(borderSize + getStepDisplaySize().width() + borderSize);
        break;
    case modes::display: {
        QSize stepDisplaySize = getStepDisplaySize();
        res.setHeight(insideBorder + ui->milestoneButton->height() + insideBorder + stepDisplaySize.height() + insideBorder);
        res.setWidth(borderSize + stepDisplaySize.width() + borderSize);
        break;
    }
    case modes::edition: {
        QSize stepDisplaySize = getStepDisplaySize();
        res.setHeight(insideBorder + ui->milestoneButton->height() + insideBorder + stepDisplaySize.height() + insideBorder + ui->newStepButton->height() + insideBorder);
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

QList<c_process *> c_milestoneView::getProcessesPtr() {
    QList<c_process *> res;
    foreach (QString key, processMap.keys()) {
        res.push_back(&processMap[key]);
        qDebug() << processMap[key].getTemperature();
    }
    return res;
}

void c_milestoneView::setDefaultMode(modes _defaultMode) {
    defaultMode = _defaultMode;
}

void c_milestoneView::slotHandleResizeStep(QAbstractAnimation *animation) {
    c_stepView *sender = static_cast<c_stepView *>(QObject::sender());
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    qsizetype indexStep = stepList.indexOf(sender);
    QPoint pos = stepList[indexStep]->pos() + QPoint(0,insideBorder + stepList[indexStep]->getSize().height());
    for (qsizetype i = indexStep+1; i < stepList.size(); ++i) {
        group->addAnimation(targetPositionAnimation(stepList[i],pos,animation->duration()));
        pos += QPoint(0,stepList[i]->getSize().height() + insideBorder);
    }
    if (mode == modes::edition) {
        group->addAnimation(targetPositionAnimation(ui->newStepButton,QPoint(borderSize,getSize(modes::edition).height()-insideBorder - ui->newStepButton->height()),animation->duration()));
    }
    group->addAnimation(targetSizeAnimation(this,getSize(mode),animation->duration()));
    group->addAnimation(animation);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void c_milestoneView::slotSwapSteps(recipe::swap direction) {
    c_stepView *sender = static_cast<c_stepView *>(QObject::sender());
    if (milestone->swapSteps(sender->getStep(),direction)) {
        qsizetype index = stepList.indexOf(sender);
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

        slotUpdateProcesses();
        switchMode(mode)->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void c_milestoneView::slotUpdateProcesses() {
    processMap.clear();
    QSet<QString> processTypesSet = c_dbManager::getProcessTypes();
    QList<QString> processTypeList = QList<QString>(processTypesSet.begin(),processTypesSet.end());
    for (int i = 0; i < processTypeList.size(); ++i) {
        processMap[processTypeList[i]] = c_process(processTypeList[i]);
    }
    for (int i = 0; i < stepList.size(); ++i) {
        c_step * step = stepList[i]->getStep();
        qDebug() << step;
        QList<c_process *> processings = step->getProcessingsPtr();
        for (int j = 0; j < processings.size(); ++j) {
            processMap[processings[j]->getType()] = *processings[j];
            qDebug() << processings[j] << processMap[processings[j]->getType()].getTemperature() << processings[j]->getType();
        }
    }
    if (processResume != nullptr) {
        processResume->setProcessings(getProcessesPtr());
    }
}

void c_milestoneView::slotAddStep() {
    stepList.push_back(new c_stepView(milestone->newStep(),this));
    stepList.last()->move(borderSize,-stepList.last()->getSize().height());
    stepList.last()->show();
    delete stepList.last()->switchMode(modes::edition,false);
    stepList.last()->lower();

    QObject::connect(stepList.last(),&c_stepView::animationRequired,this,&c_milestoneView::slotHandleResizeStep);
    QObject::connect(stepList.last(),&c_stepView::swapRank,this,&c_milestoneView::slotSwapSteps);
    QObject::connect(stepList.last(),&c_stepView::toDelete,this,&c_milestoneView::slotDeleteSteps);
    QObject::connect(stepList.last(),&c_stepView::saved,this,&c_milestoneView::slotUpdateProcesses);

    switchMode(mode,true,500)->start(QAbstractAnimation::DeleteWhenStopped);
}

void c_milestoneView::slotUpdateCurrentCharCount() {
    int max = (int(double(ui->milestoneButton->width())*0.6))/QFontMetrics(ui->milestoneNameEdit->font()).averageCharWidth();
    ui->charCount->setText(QString("%1/%2").arg(ui->milestoneNameEdit->text().size()).arg(max));
    ui->milestoneNameEdit->setMaxLength(max);
}
