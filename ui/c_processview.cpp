#include "c_processview.h"
#include "ui_c_processview.h"
#include "c_stepview.h"

int c_processView::maxNumberProcess = 3;

c_processView::c_processView(QList<c_process *> _processes, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::c_processView), processes(_processes) {
    ui->setupUi(this);
    while (processes.size() > maxNumberProcess) {
        processes.pop_back();
    }
    for (int i = 0; i < processes.size(); ++i) {
        processViews.push_back(new c_processElemView(processes[i],this));
    }
    for (int i = processes.size(); i < maxNumberProcess; ++i) {
        processViews.push_back(new c_processElemView(nullptr,this));
    }

}

c_processView::~c_processView() {
    delete ui;
}

QList<QPropertyAnimation *> c_processView::switchMode(int target, bool animated, int time) {
    QList<QPropertyAnimation *> res;
    switch (target) {
    case recipe::modes::resume:
    case recipe::modes::display: {
        QPoint pos = QPoint(0,0);
        for (int i = 0; i < processViews.size(); ++i) {
            if (!processViews[i]->isEmpty()) {
                res.append(processViews[i]->switchMode(target));
                if (animated) {
                    res.push_back(recipe::targetPositionAnimation(processViews[i],pos,time));
                } else {
                    processViews[i]->move(pos);
                }
                pos += QPoint(processViews[i]->getSize(target).width() + c_stepView::interImageSpace,0);
            }
        }
        ui->label->hide();
    }
        break;
    case recipe::modes::edition: {
        QPoint pos = QPoint(0,0);
        ui->label->move(pos);
        ui->label->show();
        pos += QPoint(0,ui->label->height() + c_stepView::interImageSpace);
        for (int i = 0; i < processViews.size(); ++i) {
            res.append(processViews[i]->switchMode(target));
            if (animated) {
                res.push_back(recipe::targetPositionAnimation(processViews[i],pos,time));
            } else {
                processViews[i]->move(pos);
            }
            pos += QPoint(0,processViews[i]->getSize(target).height() + c_stepView::interImageSpace);
        }
    }
        break;
    default:
        break;
    }
    return res;
}

QSize c_processView::getSize(int mode) {
    QSize res;
    int totalWidth = 0;
    int totalHeight = 0;
    switch (mode) {
        case recipe::modes::display:
        case recipe::modes::resume:
            for (int i = 0; i < processViews.size(); ++i) {
                totalWidth += processViews[i]->getSize(recipe::modes::display).width();
            }
            totalWidth += (processViews.size()-1)*c_stepView::interImageSpace;
            if (!isEmpty()) {
                totalHeight = processViews[0]->getSize(recipe::modes::display).height();
            }
            break;
        case recipe::modes::edition:
            for (int i = 0; i < processViews.size(); ++i) {
                totalHeight += processViews[i]->getSize(recipe::modes::display).width();
            }
            totalHeight += (processViews.size()-1)*c_stepView::interImageSpace;
            totalWidth = static_cast<c_stepView *>(parent())->width()/2 - c_stepView::borderSize - c_stepView::interImageSpace;
            break;
    default:
        break;
    }
    res.setHeight(totalHeight);
    res.setWidth(totalWidth);
    return res;
}

bool c_processView::isEmpty() {
    for (int i = 0; i < processViews.size(); ++i) {
        if(!processViews.isEmpty())
            return false;
    }
    return true;
}
