#include "c_processview.h"
#include "ui_c_processview.h"
#include "c_stepview.h"

int c_processView::maxNumberProcess = 3;

c_processView::c_processView(QList<c_process *> _processes, c_widget *widget, QWidget *parent) :
    c_widget(parent,widget),
    ui(new Ui::c_processView), processes(_processes) {
    ui->setupUi(this);
    while (processes.size() > maxNumberProcess) {
        processes.pop_back();
    }
    for (int i = 0; i < processes.size(); ++i) {
        processElems.push_back(new c_processElemView(processes[i],this));
    }
    for (int i = processes.size(); i < maxNumberProcess; ++i) {
        processElems.push_back(new c_processElemView(nullptr,this));
    }

    for (int i = 0; i < processElems.size(); ++i) {
        QObject::connect(processElems[i],&c_processElemView::removeProcess, [=] (const c_process *process) {
            static_cast<c_stepView *>(m_parent)->getStep()->removeProcessing(process);
        });
    }

    ui->label->setFixedHeight(labelHeight);

    mode = modes::resume;
}

c_processView::~c_processView() {
    delete ui;
}

QAbstractAnimation *c_processView::switchMode(modes target, bool animated, int time) {
    QParallelAnimationGroup *res =  new QParallelAnimationGroup();
    switch (target) {
    case modes::resume:
    case modes::display:
    case modes::minimal:{
        QPoint pos = QPoint(0,0);
        for (int i = 0; i < processElems.size(); ++i) {
            if (animated && mode != target) {
                res->addAnimation(slideAndDeployAnimation(processElems[i],pos,time,[=] () {
                    processElems[i]->switchMode(target);
                    processElems[i]->show();
                },mode));
            } else {
                processElems[i]->switchMode(target);
                processElems[i]->move(pos);
            }
            pos += QPoint(processElems[i]->getSize(target).width() + c_stepView::interImageSpace,0);

        }
        if (animated) {
            res->addAnimation(targetSizeAnimation(this,getSize(target),time));
            res->addAnimation(fadeAnimation(ui->label,false,time/2));
        } else {
            this->setFixedSize(getSize(target));
            ui->label->hide();
        }
    }
        break;
    case modes::edition: {
        QPoint pos = QPoint(0,0);
        processSave = processes;
        pos += QPoint(0,ui->label->height() + c_stepView::interImageSpace);
        for (int i = 0; i < processElems.size(); ++i) {
            if (animated && mode != target) {
                res->addAnimation(slideAndDeployAnimation(processElems[i],pos,time,[=] () {
                    processElems[i]->switchMode(target);
                    processElems[i]->show();
                },target));
            } else {
                processElems[i]->switchMode(target);
                processElems[i]->move(pos);
            }
            pos += QPoint(0,processElems[i]->getSize(target).height() + c_stepView::interImageSpace);
        }
        if (animated) {
            res->addAnimation(targetPositionAnimation(ui->label,QPoint(0,0),time));
            res->addAnimation(targetSizeAnimation(this,getSize(target),time));
            res->addAnimation(fadeAnimation(ui->label,true,time,time/2));
        } else {
            ui->label->move(QPoint(0,0));
            this->setFixedSize(getSize(target));
            ui->label->show();
        }
    }
        break;
    default:
        break;
    }
    mode = target;
    return res;
}

QSize c_processView::getSize(modes target) const {
    QSize res;
    int totalWidth = 0;
    int totalHeight = 0;
    switch (target) {
        case modes::display:
        case modes::resume:
        case modes::minimal:
            if (isEmpty()) {
                return QSize(0,0);
            }
            for (int i = 0; i < processElems.size(); ++i) {
                totalWidth += processElems[i]->getSize(target).width();
            }
            totalWidth += (processElems.size()-1)*c_stepView::interImageSpace;
            if (!isEmpty()) {
                totalHeight = processElems[0]->getSize(target).height();
            }
            break;
        case modes::edition:
            for (int i = 0; i < processElems.size(); ++i) {
                totalHeight += processElems[i]->getSize(target).height();
            }
            totalHeight += maxNumberProcess*c_stepView::interImageSpace + ui->label->height();
            totalWidth = static_cast<c_stepView *>(m_parent)->width()/2 - c_stepView::borderSize - c_stepView::interImageSpace;
            break;
    default:
        break;
    }
    res.setHeight(totalHeight);
    res.setWidth(totalWidth);
    return res;
}

int c_processView::getWidth(modes target) const {
    int totalWidth = 0;
    switch (target) {
        case modes::display:
        case modes::resume:
        case modes::minimal:
            for (int i = 0; i < processElems.size(); ++i) {
                totalWidth += processElems[i]->getSize(target).width();
            }
            totalWidth += (processElems.size()-1)*c_stepView::interImageSpace;
            break;
        case modes::edition:
            totalWidth = static_cast<c_stepView *>(m_parent)->width()/2 - c_stepView::borderSize - c_stepView::interImageSpace;
            break;
    default:
        break;
    }

    return totalWidth;
}

void c_processView::save() {
    processSave.clear();
    for (int i = 0; i < processElems.size(); ++i) {
        processElems[i]->save();
    }
}

void c_processView::rollback() {
    processes = processSave;
    for (int i = 0; i < processes.size(); ++i) {
        processElems[i]->setProcess(processes[i]);
    }
    for (int i = processes.size(); i < maxNumberProcess; ++i) {
        processElems[i]->setProcess(nullptr);
    }
}

c_process *c_processView::newProcessing() {
    return static_cast<c_stepView *>(m_parent)->getStep()->newProcessing();
}

bool c_processView::isEmpty() const {
    for (int i = 0; i < processElems.size(); ++i) {
        if(!processElems.isEmpty())
            return false;
    }
    return true;
}
