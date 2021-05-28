#include "c_componentview.h"
#include "ui_c_componentview.h"
#include "c_stepview.h"

c_componentView::c_componentView(QList<c_component *> _components, c_widget *widget, QWidget *parent, QString _name) :
    c_widget(parent,widget),
    ui(new Ui::c_componentView), name(_name) {
    ui->setupUi(this);

    componentSet = QSet<c_component*>(_components.begin(),_components.end());

    for (int i = 0; i < _components.size(); ++i) {
        componentsViews.push_back(new c_componentElemView(_components[i],this,ui->widget));
        componentMapView[_components[i]] = componentsViews.last();
        QObject::connect(componentsViews.last(),&c_componentElemView::deleteMe,this,&c_componentView::removeComponent);
    }

    addComponentButton = new QPushButton("Nouvel Ingrédient",ui->widget);
    addComponentButton->setFixedHeight(21);

    QObject::connect(addComponentButton,&QPushButton::clicked,this,&c_componentView::newComponent);

    enableResize = true;

    ui->labelIngredient->setFixedHeight(c_widget::labelHeight);
    if (!name.isEmpty()) {
        ui->labelIngredient->setText(name);
    }

    c_componentView::switchMode(modes::minimal,false);
}

c_componentView::~c_componentView() {
    delete ui;
}

QAbstractAnimation *c_componentView::switchMode(modes target, bool animated, int time, QAnimationGroup *parentGroupAnimation) {
    QParallelAnimationGroup *group = new QParallelAnimationGroup();
    QSize widgetSize = c_componentView::getSize(target);
    switch (target) {
        case modes::display:
        case modes::resume:
        case modes::minimal: {
            QPoint pos(insideBorder,0);
            ui->labelIngredient->move(pos);
            pos += QPoint(0,ui->labelIngredient->height() + c_stepView::interImageSpace);
            for (int i = 0; i < componentsViews.size(); ++i) {
                if (animated) {
                    group->addAnimation(targetPositionAnimation(componentsViews[i],pos,time));
                } else {
                    componentsViews[i]->move(pos);
                }
                componentsViews[i]->switchMode(target,animated,time,group);
                pos += QPoint(0,componentsViews[i]->getSize(target).height());
            }
            int max = static_cast<c_stepView *>(m_parent)->width()/2-c_stepView::borderSize - c_stepView::interImageSpace;
            addComponentButton->setFixedWidth(max - 2*insideBorder);
            if (animated) {
                group->addAnimation(targetSizeAnimation(this,widgetSize,time));
                group->addAnimation(targetPositionAnimation(addComponentButton,QPoint(insideBorder,getSize(mode).height()),time/3));
            } else {
                this->setFixedSize(widgetSize);
                addComponentButton->hide();
            }
            ui->labelIngredient->setFixedSize(widgetSize.width(),labelHeight);

        }
        break;
        case modes::edition: {
            QPoint pos(insideBorder,0);
            ui->labelIngredient->move(pos);
            pos += QPoint(0,ui->labelIngredient->height() + c_stepView::interImageSpace);
            if (mode != target) {
                 addComponentButton->move(QPoint(insideBorder,getSize(target).height()));
                 addComponentButton->lower();
            }
            for (int i = 0; i < componentsViews.size(); ++i) {
                if (animated) {
                    group->addAnimation(targetPositionAnimation(componentsViews[i],pos,time));
                } else {
                    componentsViews[i]->move(pos);
                }
                componentsViews[i]->switchMode(target,animated,time,group);
                pos += QPoint(0,componentsViews[i]->getSize(target).height() + c_stepView::interImageSpace);
            }
            int max = static_cast<c_stepView *>(m_parent)->width()/2-c_stepView::borderSize - c_stepView::interImageSpace;
            addComponentButton->setFixedWidth(max - 2*insideBorder);

            if (animated) {
                group->addAnimation(targetSizeAnimation(this,getSize(target),time));
                if (target != mode && !componentsViews.isEmpty())
                    group->addAnimation(targetPositionAnimation(addComponentButton,QPoint(insideBorder,getSize(target).height()-addComponentButton->height()),time + time/3,time));
                else
                    group->addAnimation(targetPositionAnimation(addComponentButton,QPoint(insideBorder,getSize(target).height()-addComponentButton->height()),time));
            } else {
                this->setFixedSize(getSize(target));
                addComponentButton->move(QPoint(insideBorder,getSize(target).height()-addComponentButton->height()));
            }
            addComponentButton->show();
        }
        break;
    default:
        break;
    }
    mode = target;

    return runBehavior(animated,group,parentGroupAnimation);
}

QSize c_componentView::getSize(modes target) const {
    QSize res;
    switch (target) {
        case modes::display:
        case modes::resume:
        case modes::minimal: {
            if (isEmpty()) {
                return QSize(0,0);
            }
            int widthMin = 0;
            int heightMin = 0;
            int max = static_cast<c_stepView *>(m_parent)->width()/2-c_stepView::borderSize - c_stepView::interImageSpace;
            int min = static_cast<c_stepView *>(m_parent)->width()/3-c_stepView::borderSize - c_stepView::interImageSpace;
            QFontMetrics metric(ui->labelIngredient->font());
            for (int i = 0; i < componentsViews.size(); ++i) {
                if (componentsViews[i]->width() > widthMin)
                    widthMin = componentsViews[i]->getSize(target).width();
                heightMin += componentsViews[i]->getSize(target).height();
            }
            widthMin = std::max(widthMin,metric.horizontalAdvance(ui->labelIngredient->text()));
            widthMin = std::min(widthMin,max);
            widthMin = std::max(min,widthMin);
            res.setWidth(widthMin);
            res.setHeight(heightMin + c_stepView::interImageSpace + ui->labelIngredient->height());
        }
        break;
        case modes::edition: {
            int heightMin = 0;
            int max = static_cast<c_stepView *>(m_parent)->width()/2-c_stepView::borderSize - c_stepView::interImageSpace;
            for (int i = 0; i < componentsViews.size(); ++i) {
                heightMin += componentsViews[i]->getSize(target).height();
            }
            res.setWidth(max);
            res.setHeight(heightMin + (int(componentsViews.size())+1)*c_stepView::interImageSpace + ui->labelIngredient->height()
                          + addComponentButton->height());
        }
        break;
    default:
        break;
    }
    return res;
}

int c_componentView::getWidth(modes target) const {
    switch (target) {
        case modes::display:
        case modes::resume:
        case modes::minimal: {
            int widthMin = 0;
            QFontMetrics metric(ui->labelIngredient->font());
            int max = static_cast<c_stepView *>(m_parent)->width()/2-c_stepView::borderSize - c_stepView::interImageSpace;
            for (int i = 0; i < componentsViews.size(); ++i) {
                if (componentsViews[i]->width() > widthMin)
                    widthMin = componentsViews[i]->getSize(target).width();
            }
            widthMin = std::max(widthMin,metric.horizontalAdvance(ui->labelIngredient->text()));
            widthMin = widthMin > max ? max : widthMin;
            return widthMin;
        }
        case modes::edition:
            return static_cast<c_stepView *>(m_parent)->width()/2-c_stepView::borderSize - c_stepView::interImageSpace;
    default:
        break;
    }
    return 0;
}

void c_componentView::save() {
    c_step *step = static_cast<c_stepView *>(m_parent)->getStep();
    QList<c_componentElemView *> componentViewCopy = componentsViews;
    for (int i = 0; i < componentViewCopy.size(); ++i) {
        componentViewCopy[i]->save();
        componentMapView[componentViewCopy[i]->getComponent()] = componentViewCopy[i];
    }
    for (int i = 0; i < toDeleteComponents.size(); ++i) {
        step->removeComponent(toDeleteComponents[i]->getComponent());
        componentMapView.remove(toDeleteComponents[i]->getComponent());
        toDeleteComponents[i]->hide();
        toDeleteComponents[i]->deleteLater();
    }
    toDeleteComponents.clear();
    addedComponents.clear();
}

void c_componentView::rollback() {
    c_step *step = static_cast<c_stepView *>(m_parent)->getStep();
    for (int i = 0; i < addedComponents.size(); ++i) {
        if (toDeleteComponents.contains(addedComponents[i])) {
            toDeleteComponents.removeOne(addedComponents[i]);
        }
        step->removeComponent(addedComponents[i]->getComponent());
        componentsViews.removeOne(addedComponents[i]);
        addedComponents[i]->hide();
        addedComponents[i]->deleteLater();
    }

    for (int i = 0; i < toDeleteComponents.size(); ++i) {
        componentsViews.push_back(toDeleteComponents[i]);
        toDeleteComponents[i]->show();
    }

    for (int i = 0; i < componentsViews.size(); ++i) {
        componentsViews[i]->rollback();
    }
    toDeleteComponents.clear();
    addedComponents.clear();
}

void c_componentView::updateComponents(QList<c_component *> newList, QAnimationGroup *parentGroupAnimation) {
    QList<c_component*> addSet = (QSet<c_component*>(newList.begin(),newList.end()) - componentSet).values();
    QList<c_component*> removeSet = (componentSet - QSet<c_component*>(newList.begin(),newList.end())).values();
    QList<c_component*> updateSet = (componentSet.intersect(QSet<c_component*>(newList.begin(),newList.end()))).values();
    for (int i = 0; i < addSet.size(); ++i) {
        componentsViews.push_back(new c_componentElemView(addSet[i],this,ui->widget));
        componentMapView[addSet[i]] = componentsViews.last();
        componentsViews.last()->show();
        componentsViews.last()->switchMode(modes::display,false);
        componentsViews.last()->move(insideBorder,-componentsViews.last()->getSize(modes::display).height());
        componentSet.insert(addSet[i]);
    }
    for (int i = 0; i < removeSet.size(); ++i) {
        c_componentElemView* toRemove = componentMapView[removeSet[i]];
        componentsViews.removeOne(toRemove);
        toRemove->hide();
        componentMapView.remove(removeSet[i]);
        componentSet.remove(removeSet[i]);
    }
    for (int i = 0; i < updateSet.size(); ++i) {
        componentMapView[updateSet[i]]->updateComponent(updateSet[i]);
    }
    switchMode(mode,true,parentGroupAnimation->duration(),parentGroupAnimation);
}

bool c_componentView::isEmpty() const {
    return componentsViews.isEmpty();
}

void c_componentView::newComponent() {
    c_step *step = static_cast<c_stepView *>(m_parent)->getStep();
    componentsViews.push_back(new c_componentElemView(step->newComponent(),this,ui->widget));
    componentsViews.last()->show();
    componentsViews.last()->setFocus();
    componentsViews.last()->lower();
    componentsViews.last()->switchMode(modes::edition,false);
    componentsViews.last()->move(insideBorder,-componentsViews.last()->getSize(modes::edition).height());
    QObject::connect(componentsViews.last(),&c_componentElemView::deleteMe,this,&c_componentView::removeComponent);

    addedComponents.append(componentsViews.last());

    if (enableResize)
        emit resized();
}

void c_componentView::removeComponent() {
    c_componentElemView *sender = static_cast<c_componentElemView *>(QObject::sender());
    componentsViews.removeOne(sender);
    toDeleteComponents.push_back(sender);
    sender->hide();

    if (enableResize)
        emit resized();
}

const QString &c_componentView::getName() const {
    return name;
}

void c_componentView::setName(const QString &newName) {
    name = newName;
}
