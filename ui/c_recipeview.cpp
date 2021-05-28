#include "c_recipeview.h"
#include "ui_c_recipeview.h"

c_recipeView::c_recipeView(c_recipe *recipe, c_widget *widget, QWidget *parent) :
    c_widget(parent,widget),
    ui(new Ui::c_recipeView), recipe(recipe) {
    ui->setupUi(this);

    heightMilstones = 0;
    componentChanged = false;
    senderComponentChanged = nullptr;

    QSet<QString> processTypesSet = c_dbManager::getProcessTypes();
    QList<QString> processTypeList = QList<QString>(processTypesSet.begin(),processTypesSet.end());
    for (int i = 0; i < processTypeList.size(); ++i) {
        processMap[processTypeList[i]] = new c_process(processTypeList[i]);
    }

    QList<c_milestone *> milestones = recipe->getPlanningPtr();
    for (int i = 0; i < milestones.size(); ++i) {
        milestonesViews.push_back(new c_milestoneView(milestones[i],this,ui->milestoneArea));
        milestonesViews.last()->setDefaultMode(modes::edition,false);
        componentMap[milestonesViews.last()] = new c_componentView(milestonesViews.last()->getComponentsList(),this,this,milestones[i]->getName());
        QMap<QString,c_process> processes = milestonesViews.last()->getProcessMap();
        foreach (QString key, processes.keys()) {
            processMap[key]->setDuration(processMap[key]->getDuration() + processes[key].getDuration());
        }
        QObject::connect(milestonesViews.last(),&c_milestoneView::componentsListChanged,this,&c_recipeView::slotComponentListChanged);
        QObject::connect(milestonesViews.last(),&c_milestoneView::resized,this, [=] () {
            switchMode(mode,true,500);
        });
    }

    globalProcessingView = new c_processView(processMap.values(),this,this);


    ui->milestoneArea->setFixedWidth(milestonesViews[0]->getSize(modes::display).width());
    ui->milestoneArea->setStyleSheet("QWidget#milestoneArea {"
                                     "  border : 1px solid black;"
                                     "}");
    ui->milestonesScroll->setFixedWidth(ui->milestoneArea->width() + 2*insideBorder + 11);
    ui->labelIngredients->setStyleSheet("font-size: 16px");
    ui->labelIngredients->setFixedHeight(22);
    ui->labelProcesses->setStyleSheet("font-size: 16px");
    ui->labelProcesses->setFixedHeight(22);


    c_recipeView::switchMode(modes::display,false);
}

c_recipeView::~c_recipeView()
{
    delete ui;
}

QAbstractAnimation *c_recipeView::switchMode(modes target, bool animated, int time, QAnimationGroup *parentGroupAnimation) {
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    QSize mainImageSize = recipe->getImageUrl().isEmpty() ? QSize(0,0) : QSize(200,200);
    QSize recipeViewSize = this->size();
    switch (target) {
    case modes::display:{
        // Main Image
        ui->imageRecipe->move(borderSize,borderSize);
        if (recipe->getImageUrl().isEmpty()) {
            ui->imageRecipe->hide();
        ui->labelIngredients->move(2*borderSize,3*borderSize+imageRecipe->getSize(target).height());
        ui->labelIngredients->setFixedWidth(getComponentsAreaWidth(target));

        // Components
        targetPos = QPoint(2*borderSize,imageRecipe->getSize().height()+ ui->labelIngredients->height() + 3*borderSize + insideBorder);
        for (int i = 0; i < milestonesViews.size(); ++i) {
            if (animated) {
                group->addAnimation(targetPositionAnimation(componentMap[milestonesViews[i]],targetPos,time));
            } else {
                componentMap[milestonesViews[i]]->move(targetPos);
            }
            targetPos += QPoint(0,componentMap[milestonesViews[i]]->getSize().height() + insideBorder);
        }

        ui->labelProcesses->move(2*borderSize + ui->labelIngredients->width() + insideBorder, 3*borderSize+imageRecipe->getSize(target).height());
         ui->labelProcesses->setFixedWidth(getProcessesAreaWidth(target));
        globalProcessingView->move(2*borderSize + ui->labelIngredients->width() + insideBorder, 3*borderSize+imageRecipe->getSize(target).height() + ui->labelIngredients->height() + insideBorder);
        // title
        ui->titleRecipe->move(mainImageSize.width() + borderSize + insideBorder, borderSize);

        // ScrollArea
        ui->milestonesScroll->move(mainImageSize.width() + borderSize + insideBorder, borderSize + ui->titleRecipe->height() + insideBorder);
        ui->milestonesScroll->setFixedHeight(recipeViewSize.height() - (2*borderSize + ui->titleRecipe->height() + insideBorder));

        // MilestoneArea
        QPoint targetPos = QPoint(0,0);
        for (int i = 0; i < milestonesViews.size(); ++i) {
            if (animated) {
                group->addAnimation(targetPositionAnimation(milestonesViews[i],targetPos,time));
            } else {
                milestonesViews[i]->move(targetPos);
            }
            targetPos += QPoint(0,insideBorder + milestonesViews[i]->getSize().height());
        }
        if (animated) {
            if (targetPos.y() > heightMilstones) {
                qDebug() << "resize";
                ui->milestoneArea->setFixedHeight(targetPos.y());
            } else {
                qDebug() << "resize later";
            }
            heightMilstones = targetPos.y();
        } else {
            ui->milestoneArea->setFixedHeight(targetPos.y());
        }


        break;
    }
    default:
        break;
    }

    mode = display;
    return c_widget::runBehavior(animated,group,parentGroupAnimation);
}

void c_recipeView::handleChildrenAnimation(QAbstractAnimation *animation) {
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    group->addAnimation(animation);
    if (componentChanged) {
        updateOneComponentsList(group);
    }
    switchMode(mode,true,group->duration(),group);
    ui->milestonesScroll->verticalScrollBar()->value();
    runBehavior(true,group,nullptr);

}

int c_recipeView::getComponentsAreaWidth(modes ) const {
    return 200;
}

int c_recipeView::getProcessesAreaWidth(modes ) const {
    return 200;
}

void c_recipeView::resizeEvent(QResizeEvent *) {
    switchMode(mode);
}

void c_recipeView::updateOneComponentsList(QAnimationGroup *parentGroupAnimation) {
    componentMap[senderComponentChanged]->updateComponents(senderComponentChanged->getComponentsList(),parentGroupAnimation);
    componentChanged = false;
    senderComponentChanged = nullptr;
}

void c_recipeView::slotComponentListChanged() {
    componentChanged = true;
    senderComponentChanged = static_cast<c_milestoneView *>(QObject::sender());
}
