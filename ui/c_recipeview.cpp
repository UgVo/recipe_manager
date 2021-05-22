#include "c_recipeview.h"
#include "ui_c_recipeview.h"

c_recipeView::c_recipeView(c_recipe *recipe, c_widget *widget, QWidget *parent) :
    c_widget(parent,widget),
    ui(new Ui::c_recipeView), recipe(recipe) {
    ui->setupUi(this);

    heightMilstones = 0;

    QList<c_milestone *> milestones = recipe->getPlanningPtr();
    for (int i = 0; i < milestones.size(); ++i) {
        milestonesViews.push_back(new c_milestoneView(milestones[i],this,ui->milestoneArea));
        milestonesViews.last()->setDefaultMode(modes::edition,false);
        QObject::connect(milestonesViews.last(),&c_milestoneView::resized,this, [=] () {
            switchMode(mode,true,500);
        });
    }

    ui->milestoneArea->setFixedWidth(milestonesViews[0]->getSize(modes::display).width());
    ui->milestoneArea->setStyleSheet("QWidget#milestoneArea {"
                                     "  border : 1px solid black;"
                                     "}");
    ui->milestonesScroll->setFixedWidth(ui->milestoneArea->width() + 2*insideBorder + 11);

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
        }

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
    switchMode(mode,true,group->duration(),group);
    ui->milestonesScroll->verticalScrollBar()->value();
    runBehavior(true,group,nullptr);

}

void c_recipeView::resizeEvent(QResizeEvent *) {
    switchMode(mode);
}
