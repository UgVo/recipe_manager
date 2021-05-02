#include "c_componentview.h"
#include "ui_c_componentview.h"
#include "c_stepview.h"

c_componentView::c_componentView(QList<c_component *> _components, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::c_componentView), components(_components) {
    ui->setupUi(this);

    QHBoxLayout* layout = static_cast<QHBoxLayout*>(ui->widget->layout());
    for (int i = 0; i < components.size(); ++i) {
        componentsViews.push_back(new c_componentElemView(components[i],this));
        layout->insertWidget(i+1,componentsViews.last());
        QObject::connect(componentsViews.last(),&c_componentElemView::deleteMe,this,&c_componentView::removeComponent);
    }

    addComponentButton = new QPushButton("Nouvel Ingrédient",this);
    addComponentButton->setFixedHeight(21);

    QObject::connect(addComponentButton,&QPushButton::clicked,this,&c_componentView::newComponent);
    switchMode(recipe::modes::resume,false);
}

c_componentView::~c_componentView() {
    delete ui;
}

QList<QPropertyAnimation *> c_componentView::switchMode(int target, bool animated, int time) {
    QList<QPropertyAnimation *> res;
    switch (target) {
        case recipe::modes::display:
        case recipe::modes::resume: {
            for (int i = 0; i < componentsViews.size(); ++i) {
                QList<QPropertyAnimation *> anims = componentsViews[i]->switchMode(recipe::modes::resume);
                if (animated) {
                    for (int i = 0; i < anims.size(); ++i) {
                        res.push_back(anims[i]);
                    }
                }
            }
            int max = static_cast<c_stepView *>(parent())->width()/2-c_stepView::borderSize - c_stepView::interImageSpace;
            QHBoxLayout* layout = static_cast<QHBoxLayout*>(ui->widget->layout());
            int left, right;
            layout->getContentsMargins(&left,nullptr,&right,nullptr);
            addComponentButton->setFixedWidth(max - left - right);
            addComponentButton->move(QPoint(left,getSize(target).height()-addComponentButton->height()));
            QSize targetSize = getSize(target);
            if (animated) {
                res.push_back(recipe::targetSizeAnimation(this,targetSize,time));
            } else {
                this->setFixedSize(targetSize);
            }

            ui->widget->setStyleSheet("QWidget#widget {"
                                      " border : 1px solid white;"
                                      " border-radius : 2px;"
                                      "}");
            addComponentButton->hide();
        }
        break;
        case recipe::modes::edition: {
            componentsSave = components;
            for (int i = 0; i < componentsViews.size(); ++i) {
                QList<QPropertyAnimation *> anims = componentsViews[i]->switchMode(recipe::modes::edition);
                if (animated) {
                    for (int i = 0; i < anims.size(); ++i) {
                        res.push_back(anims[i]);
                    }
                }
            }
            int max = static_cast<c_stepView *>(parent())->width()/2-c_stepView::borderSize - c_stepView::interImageSpace;
            QHBoxLayout* layout = static_cast<QHBoxLayout*>(ui->widget->layout());
            int left, right;
            layout->getContentsMargins(&left,nullptr,&right,nullptr);
            addComponentButton->setFixedWidth(max - left - right);

            if (animated) {
                res.push_back(recipe::targetSizeAnimation(this,getSize(target),time));
                res.push_back(recipe::targetPositionAnimation(addComponentButton,QPoint(left,getSize(target).height()-addComponentButton->height()),time));
            } else {
                this->setFixedSize(getSize(target));
                addComponentButton->move(QPoint(left,getSize(target).height()-addComponentButton->height()));
            }

            ui->widget->setStyleSheet("QWidget#widget {"
                                      " border : 1px solid white;"
                                      " border-radius : 2px;"
                                      "}");
            addComponentButton->show();
        }
        break;
    default:
        break;
    }
    return res;
}

QSize c_componentView::getSize(int mode) {
    QSize res;
    switch (mode) {
        case recipe::modes::display:
        case recipe::modes::resume: {
            int widthMin = 0;
            int heightMin = 0;
            int max = static_cast<c_stepView *>(parent())->width()/2-c_stepView::borderSize - c_stepView::interImageSpace;
            QFontMetrics metric(ui->label_2->font());
            QHBoxLayout* layout = static_cast<QHBoxLayout*>(ui->widget->layout());
            for (int i = 0; i < componentsViews.size(); ++i) {
                if (componentsViews[i]->width() > widthMin)
                    widthMin = componentsViews[i]->width();
                heightMin += componentsViews[i]->height();
            }
            widthMin = std::max(widthMin,metric.horizontalAdvance(ui->label_2->text()));
            widthMin = widthMin > max ? max : widthMin;
            res.setWidth(widthMin);
            res.setHeight(heightMin + components.size()*layout->spacing() + ui->label_2->height());
        }
        break;
        case recipe::modes::edition: {
            int heightMin = 0;
            int top, bottom;
            int max = static_cast<c_stepView *>(parent())->width()/2-c_stepView::borderSize - c_stepView::interImageSpace;
            QFontMetrics metric(ui->label_2->font());
            QHBoxLayout* layout = static_cast<QHBoxLayout*>(ui->widget->layout());
            layout->getContentsMargins(nullptr,&top,nullptr,&bottom);
            for (int i = 0; i < componentsViews.size(); ++i) {
                heightMin += componentsViews[i]->height();
            }
            res.setWidth(max);
            res.setHeight(heightMin + (components.size()+2)*layout->spacing() + ui->label_2->height()
                          + addComponentButton->height() + top + bottom);
        }
        break;
    default:
        break;
    }
    return res;
}

void c_componentView::save() {
    for (int i = 0; i < componentsViews.size(); ++i) {
        componentsViews[i]->save();
    }
}

void c_componentView::newComponent() {
    QHBoxLayout* layout = static_cast<QHBoxLayout*>(ui->widget->layout());
    c_step *step = static_cast<c_stepView *>(parent())->getStep();
    components.push_back(step->newComponent());
    componentsViews.push_back(new c_componentElemView(components.last(),this));
    layout->insertWidget(componentsViews.size(),componentsViews.last());
    componentsViews.last()->setFocus();
    QObject::connect(componentsViews.last(),&c_componentElemView::deleteMe,this,&c_componentView::removeComponent);

    emit resized();
}

void c_componentView::removeComponent() {
    c_step *step = static_cast<c_stepView *>(parent())->getStep();
    c_componentElemView *sender = static_cast<c_componentElemView *>(QObject::sender());
    c_component *component = sender->getComponent();
    step->removeComponent(component);
    components.removeOne(component);
    componentsViews.removeOne(sender);
    sender->hide();
    sender->deleteLater();

    emit resized();
}
