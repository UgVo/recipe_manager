#include "c_milestoneview.h"
#include "ui_c_milestoneview.h"

c_milestoneView::c_milestoneView(c_milestone *_milestone, c_widget *widget, QWidget *parent) :
    c_widget(parent,widget),
    ui(new Ui::c_milestoneView), milestone(_milestone) {
    ui->setupUi(this);

    defaultMode = modes::display;

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
    ui->milestoneNameEdit->setStyleSheet("QLineEdit {"
                                         "  background: transparent;"
                                         "  border-width : 0px 0px 1px 0px; "
                                         "  border-style: solid; "
                                         "  border-color: black;"
                                         "  margin-bottom: 3px;"
                                         "  margin-top: 3px;"
                                         "}");
    QObject::connect(ui->milestoneNameEdit,&QLineEdit::textChanged,this,&c_milestoneView::slotUpdateCurrentCharCount);

    scene = new QGraphicsScene(ui->milestoneButton);
    view = new QGraphicsView(scene,ui->milestoneButton);
    QPixmap pix = QPixmap(":/images/collapse-arrow.png").scaled(ui->milestoneButton->height(),ui->milestoneButton->height()-2,Qt::KeepAspectRatio,Qt::SmoothTransformation);
    view->setFixedSize(pix.width()+2,pix.height());
    view->move(1,1);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setStyleSheet("background: transparent;"
                        "border : 0px solid white;");
    item = new c_pixmapGraphics();
    item->setPixmap(pix);
    item->setTransformOriginPoint(double(pix.width())/2.0,double(pix.height())/2.0);
    item->setTransformationMode(Qt::SmoothTransformation);
    view->viewport()->installEventFilter(this);


    scene->addItem(item);
    view->show();

    QObject::connect(ui->milestoneButton,&QPushButton::clicked, [=] () {
        switch (mode) {
        case modes::edition:
        case modes::display: {
            QParallelAnimationGroup *group = new QParallelAnimationGroup;
            switchMode(minimal,true,500,group);
            group->start(QAbstractAnimation::DeleteWhenStopped);
            break;
        }
        case modes::minimal: {
            QParallelAnimationGroup *group = new QParallelAnimationGroup;
            switchMode(defaultMode,true,500,group);
            group->start(QAbstractAnimation::DeleteWhenStopped);
            break;
        }
        default:
            switchMode(modes::minimal,true,500);
            break;
        }
    });

    this->setFixedWidth(stepWidth + borderSize*2);

    for (int i = 0; i < steps.size(); ++i) {
        stepList.push_back(new c_stepView(steps[i],this,this));
        QObject::connect(stepList.last(),&c_stepView::animationRequired,this,&c_milestoneView::slotHandleResizeStep);
        QObject::connect(stepList.last(),&c_stepView::swapRank,this,&c_milestoneView::slotSwapSteps);
        QObject::connect(stepList.last(),&c_stepView::toDelete,this,&c_milestoneView::slotDeleteSteps);
        QObject::connect(stepList.last(),&c_stepView::saved,this,&c_milestoneView::slotUpdateProcesses);
        QObject::connect(stepList.last(),&c_stepView::saved,this,&c_milestoneView::slotUpdateComponentsList);
    }

    processResume = nullptr;

    slotUpdateProcesses();

    processResume = new c_processView(getProcessesPtr(),this,ui->milestoneButton);
    processResume->switchMode(modes::minimal,false);

    this->setStyleSheet("outline : 0;");

    slotUpdateComponentsList();

    mode = modes::minimal;
    c_milestoneView::switchMode(mode,false);
}

c_milestoneView::~c_milestoneView() {
    delete ui;
    if (scene != nullptr)
        delete scene;
    if (view != nullptr)
        delete view;
    if (item != nullptr)
        delete item;
}

QAbstractAnimation *c_milestoneView::switchMode(c_widget::modes target, bool animated, int time, QAnimationGroup *parentGroupAnimation) {
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    QSize processesTargetSize = processResume->getSize(modes::minimal);
    QSize stepTargetSize = this->getSize(target);
    QSize stepTargetSizeEdition = this->getSize(modes::edition);
    switch (target) {
    case modes::minimal: {
        if (target != mode) {
            if (animated) {
                group->addAnimation(rotateAnimation(item,0.0,time));
            } else {
                item->setRotation(0.0);
            }
        }

        view->move(ui->milestoneButton->width() - view->width()-insideBorder,1);
        if (animated) {
            group->addAnimation(targetSizeAnimation(this,stepTargetSize,time));
        } else {
            this->setFixedSize(stepTargetSize);
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

        if (mode != target) {
            pos = QPoint( ui->milestoneButton->width() - view->width() - borderSize - processesTargetSize.width(),ui->milestoneButton->height());
            processResume->move(pos);
        }
        pos = QPoint( ui->milestoneButton->width() - view->width() - borderSize - processesTargetSize.width(),insideBorder);
        if (animated) {
            group->addAnimation(targetPositionAnimation(processResume,pos,time));
        } else {
            processResume->move(pos);
        }

        pos = QPoint(borderSize,stepTargetSizeEdition.height());
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
        if (target != mode) {
            if (animated) {
                group->addAnimation(rotateAnimation(item,-180.0,time));
            } else {
                item->setRotation(0.0);
            }
        }

        view->move(ui->milestoneButton->width() - view->width()-insideBorder,1);
        if (animated) {
            group->addAnimation(targetSizeAnimation(this,stepTargetSize,time));
        } else {
            this->setFixedSize(stepTargetSize);
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

        pos = QPoint( ui->milestoneButton->width() - view->width() - borderSize - processesTargetSize.width(),ui->milestoneButton->height());
        if (animated) {
            group->addAnimation(targetPositionAnimation(processResume,pos,time));
        } else {
            processResume->move(pos);
        }

        pos = QPoint(borderSize,stepTargetSizeEdition.height());
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
        if (target != mode) {
            if (animated) {
                group->addAnimation(rotateAnimation(item,-180.0,time));
            } else {
                item->setRotation(0.0);
            }
        }

        view->move(ui->milestoneButton->width() - view->width()-insideBorder,1);
        if (animated) {
            group->addAnimation(targetSizeAnimation(this,stepTargetSize,time));
        } else {
            this->setFixedSize(stepTargetSize);
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
        pos = QPoint( ui->milestoneButton->width() - view->width() - borderSize - processesTargetSize.width(),ui->milestoneButton->height());
        if (animated) {
            group->addAnimation(targetPositionAnimation(processResume,pos,time));
        } else {
            processResume->move(pos);
        }

        ui->milestoneNameEdit->show();
        QSize targetSize(std::min(pos.x() - (insideBorder + borderSize - 2) - insideBorder,int(double(width())*0.6)),ui->milestoneNameEdit->height());
        pos = QPoint(insideBorder + borderSize - 1,insideBorder + (ui->milestoneButton->height() - ui->milestoneNameEdit->height())/2);
        ui->milestoneNameEdit->move(pos);
        if (mode != target) {
            ui->milestoneNameEdit->setText("");
            ui->milestoneNameEdit->setFixedWidth(0);
            QObject::connect(group,&QAbstractAnimation::finished,group,[=] () {
                slotUpdateCurrentCharCount();
                ui->milestoneNameEdit->setText(ui->milestoneButton->text());
                ui->milestoneButton->setText("");
                ui->charCount->move(QPoint(targetSize.width() + borderSize + insideBorder - 2 - ui->charCount->width(),insideBorder + (ui->milestoneButton->height() - ui->charCount->height())/2));
                ui->charCount->show();
            });
        }
        if (animated) {
            group->addAnimation(targetSizeAnimation(ui->milestoneNameEdit,targetSize,time));
        } else {
            ui->milestoneNameEdit->setFixedSize(targetSize);
        }

        pos = QPoint(borderSize,stepTargetSizeEdition.height()-insideBorder - ui->newStepButton->height());
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
    emit resized();

    return runBehavior(animated,group,parentGroupAnimation);
}

QSize c_milestoneView::getSize(modes target) const {
    QSize res;
    if (target == modes::none)
        target = mode;
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

void c_milestoneView::resizeEvent(QResizeEvent *) {
    //repaint();
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
    }
    return res;
}

void c_milestoneView::setDefaultMode(modes _defaultMode, bool animated) {
    defaultMode = _defaultMode;
    if (animated) {
        QParallelAnimationGroup *group = new QParallelAnimationGroup;
        for (int i = 0; i < stepList.size(); ++i) {
            stepList[i]->switchMode(modes::none,true,500,group);
        }
        group->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        for (int i = 0; i < stepList.size(); ++i) {
            stepList[i]->switchMode(modes::none,false);
        }
    }
}

void c_milestoneView::slotHandleResizeStep() {
    switchMode(mode,true,500);
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
    switchMode(mode,true,500);
}

void c_milestoneView::slotDeleteSteps() {
    c_stepView *sender = static_cast<c_stepView *>(QObject::sender());
    if (milestone->removeStep(sender->getStep())) {
        stepList.removeOne(sender);
        sender->hide();
        sender->deleteLater();

        slotUpdateProcesses();
        switchMode(mode,true,500);
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
        QList<c_process *> processings = step->getProcessingsPtr();
        for (int j = 0; j < processings.size(); ++j) {
            processMap[processings[j]->getType()].setDuration(processings[j]->getDuration() + processMap[processings[j]->getType()].getDuration());
        }
    }
    if (processResume != nullptr) {
        processResume->setProcessings(getProcessesPtr());
    }
}

void c_milestoneView::slotAddStep() {
    stepList.push_back(new c_stepView(milestone->newStep(),this,this));
    stepList.last()->move(borderSize,-stepList.last()->getSize().height());
    stepList.last()->show();
    stepList.last()->switchMode(modes::edition,false);
    stepList.last()->lower();

    QObject::connect(stepList.last(),&c_stepView::animationRequired,this,&c_milestoneView::slotHandleResizeStep);
    QObject::connect(stepList.last(),&c_stepView::swapRank,this,&c_milestoneView::slotSwapSteps);
    QObject::connect(stepList.last(),&c_stepView::toDelete,this,&c_milestoneView::slotDeleteSteps);
    QObject::connect(stepList.last(),&c_stepView::saved,this,&c_milestoneView::slotUpdateProcesses);

    switchMode(mode,true,500);
}

void c_milestoneView::slotUpdateCurrentCharCount() {
    int max = (int(double(ui->milestoneButton->width())*0.6))/QFontMetrics(ui->milestoneNameEdit->font()).averageCharWidth();
    ui->charCount->setText(QString("%1/%2").arg(ui->milestoneNameEdit->text().size()).arg(max));
    ui->milestoneNameEdit->setMaxLength(max);
}

c_milestone *c_milestoneView::getMilestone() const
{
    return milestone;
}

const QMap<QString, c_component *> &c_milestoneView::getComponents() const {
    return componentsList;
}

QList<c_component *> c_milestoneView::getComponentsList() {
    return componentsList.values();
}

void c_milestoneView::setComponentsList(const QMap<QString, c_component *> &newComponentsList) {
    if (componentsList == newComponentsList)
        return;
    componentsList = newComponentsList;
    emit componentsListChanged();
}

c_widget::modes c_milestoneView::getDefaultMode() const {
    return defaultMode;
}

bool c_milestoneView::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseButtonRelease) {
        if (static_cast<QMouseEvent *>(event)->button() == Qt::LeftButton) {
            ui->milestoneButton->click();
            return true;
        } else {
            return QObject::eventFilter(obj, event);
        }
    }
    return QObject::eventFilter(obj, event);
}

void c_milestoneView::handleChildrenAnimation(QAbstractAnimation *animation) {
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    group->addAnimation(animation);
    switchMode(mode,true,group->duration(),group);
    runBehavior(true,group,nullptr);
}

void c_milestoneView::slotUpdateComponentsList() {
    componentsList.clear();
    QList<c_step *> steps = milestone->getStepsPtr();
    for (int i = 0; i < steps.size(); ++i) {
        QList<c_component *> compo = steps[i]->getComponentsPtr();
        for (int j = 0; j < compo.size(); ++j) {
            QString key = QString("%1%2").arg(compo[j]->getIngredient().getName(),
                                              recipe::unitToString[compo[j]->getUnit()]);
            if (componentsList.contains(key)) {
                *componentsList[key] += *compo[j];
            } else {
                componentsList[key] = compo[j];
            }
        }
    }
    emit componentsListChanged();
}

const QMap<QString, c_process> &c_milestoneView::getProcessMap() const
{
    return processMap;
}

void c_milestoneView::setProcessMap(const QMap<QString, c_process> &newProcessMap)
{
    if (processMap == newProcessMap)
        return;
    processMap = newProcessMap;
    emit processMapChanged();
}
