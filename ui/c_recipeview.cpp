#include "c_recipeview.h"
#include "ui_c_recipeview.h"

c_recipeView::c_recipeView(c_recipe *recipe, c_widget *widget, QWidget *parent) :
    c_widget(parent,widget),
    ui(new Ui::c_recipeView), recipe(recipe) {
    ui->setupUi(this);

    heightMilstones = 0;
    componentChanged = false;
    equipmentSetChanged = false;
    senderComponentChanged = nullptr;
    equipments = nullptr;

    QSet<QString> processTypesSet = c_dbManager::getProcessTypes();
    QList<QString> processTypeList = QList<QString>(processTypesSet.begin(),processTypesSet.end());
    for (int i = 0; i < processTypeList.size(); ++i) {
        processMap[processTypeList[i]] = new c_process(processTypeList[i]);
    }

    ui->labelIngredients->setStyleSheet("font-size: 16px");
    ui->labelIngredients->setFixedHeight(22);
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
        QObject::connect(milestonesViews.last(),&c_milestoneView::processMapChanged,this,&c_recipeView::slotProcessMapChanged);
        QObject::connect(milestonesViews.last(),&c_milestoneView::equipmentListChanged,this,&c_recipeView::slotEquipmentSetChanged);
        QObject::connect(milestonesViews.last(),&c_milestoneView::resized,this, [=] () {
            switchMode(mode,true,500);
        });
        milestonesViews.last()->updateEquipmentSet();
    }

    slotEquipmentSetChanged();

    ui->milestoneArea->setFixedWidth(milestonesViews[0]->getSize(modes::display).width());
    ui->milestoneArea->setStyleSheet("QWidget#milestoneArea {"
                                     "  border : 1px solid black;"
                                     "}");
    ui->milestonesScroll->setFixedWidth(ui->milestoneArea->width() + 2*insideBorder + 11);
    ui->titleRecipe->setFixedHeight(26);
    ui->titleRecipe->setFixedWidth(ui->milestonesScroll->width());
    ui->titleRecipe->setStyleSheet("font-size: 20px;");


    imageRecipe = new c_image(recipe->getImageUrl(),this,this);
    imageRecipe->switchMode(modes::display,false);

    ui->labelProcesses->setStyleSheet("font-size: 16px");
    ui->labelProcesses->setFixedHeight(22);
    globalProcessingView = new c_processView(processMap.values(),this,this);

    ui->labelEquipment->setStyleSheet("font-size: 16px");
    ui->labelEquipment->setFixedHeight(22);
    equipments = new c_equipementsView(equipmentSet.values(),this,this);
    equipments->hideTitle(true);

    //this->setStyleSheet("border : 1px solid black");

    c_recipeView::switchMode(modes::display,false);
}

c_recipeView::~c_recipeView()
{
    delete ui;
}

QAbstractAnimation *c_recipeView::switchMode(modes target, bool animated, int time, QAnimationGroup *parentGroupAnimation) {
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    QSize recipeViewSize = this->size();
    int leftWidth = std::max(getImageAreaWidth(target) + 4*borderSize, getEquipmentsAreaWidth(target) + getComponentsAreaWidth(target) + 4*borderSize);
    QPoint targetPos;
    switch (target) {
    case modes::display:{
        // Main Image
        imageRecipe->move(2*borderSize,2*borderSize);
        if (recipe->getImageUrl().isEmpty()) {
            imageRecipe->hide();
        }
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

        // Processes
        ui->labelProcesses->move(2*borderSize + ui->labelIngredients->width() + insideBorder, 3*borderSize+imageRecipe->getSize(target).height());
        ui->labelProcesses->setFixedWidth(getProcessesAreaWidth(target));
        globalProcessingView->move(2*borderSize + ui->labelIngredients->width() + insideBorder, 3*borderSize+imageRecipe->getSize(target).height() + ui->labelIngredients->height() + insideBorder);

        // Equipments
        ui->labelEquipment->move(globalProcessingView->pos() + QPoint(0,globalProcessingView->getSize(target).height() + borderSize));
        ui->labelEquipment->setFixedWidth(getEquipmentsAreaWidth(target));
        equipments->move(ui->labelEquipment->pos() + QPoint(0,ui->labelEquipment->height() + insideBorder));

        // title
        ui->titleRecipe->move(leftWidth, 2*borderSize);
        ui->titleRecipe->setText(recipe->getName());
        ui->titleRecipeEdit->hide();

        // ScrollArea
        ui->milestonesScroll->move(leftWidth, 2*borderSize + ui->titleRecipe->height() + insideBorder);
        ui->milestonesScroll->setFixedHeight(recipeViewSize.height() - (3*borderSize + ui->titleRecipe->height() + borderSize));

        // MilestoneArea
        targetPos = QPoint(0,0);
        for (int i = 0; i < milestonesViews.size(); ++i) {
            if (animated) {
                group->addAnimation(targetPositionAnimation(milestonesViews[i],targetPos,time));
            } else {
                milestonesViews[i]->move(targetPos);
            }
            targetPos += QPoint(0,insideBorder + milestonesViews[i]->getSize().height());
        }
        if (targetPos.y() > heightMilstones) {
            ui->milestoneArea->setFixedHeight(targetPos.y());
        }
        heightMilstones = targetPos.y();


        this->setFixedWidth(leftWidth + ui->milestonesScroll->width() + 2*borderSize);

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
    if (equipmentSetChanged) {
        updateEquipmentView(group);
    }
    switchMode(mode,true,group->duration(),group);
    runBehavior(true,group,nullptr);
}

int c_recipeView::getImageAreaWidth(modes ) const {
    return 400;
}

int c_recipeView::getEquipmentsAreaWidth(modes ) const {
    return 200;
}

int c_recipeView::getEquipmentsAreaHeight(modes target) const {
    return height() - (3*borderSize+imageRecipe->getSize(target).height()
                       + borderSize + ui->labelProcesses->height() + borderSize
                       + globalProcessingView->getSize(target).height() + borderSize
                       + ui->labelEquipment->height() + borderSize);
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

void c_recipeView::updateEquipmentView(QAnimationGroup *parentGroupAnimation) {
    if (equipments != nullptr) {
        equipments->setEquipmentList(equipmentSet);
        equipments->switchMode(modes::resume,true,parentGroupAnimation->duration(),parentGroupAnimation);
    }
    equipmentSetChanged = false;
}

void c_recipeView::slotComponentListChanged() {
    componentChanged = true;
    senderComponentChanged = static_cast<c_milestoneView *>(QObject::sender());
}

void c_recipeView::slotProcessMapChanged() {
    for (auto it = processMap.begin(); it != processMap.end(); ++it) {
        *it.value() = c_process(it.key());
    }
    for (int i = 0; i < milestonesViews.size(); ++i) {
        QList<c_process> processings = milestonesViews[i]->getProcessMap().values();
        for (int j = 0; j < processings.size(); ++j) {
            processMap[processings[j].getType()]->setDuration(processings[j].getDuration() + processMap[processings[j].getType()]->getDuration());
        }
    }
    globalProcessingView->updateProcessings();
}

void c_recipeView::slotEquipmentSetChanged() {
    equipmentSet.clear();
    for (c_milestoneView * elem : milestonesViews) {
        equipmentSet += elem->getEquipmentSet();
    }
    equipmentSetChanged = true;
}
