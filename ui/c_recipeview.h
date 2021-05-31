#ifndef C_RECIPEVIEW_H
#define C_RECIPEVIEW_H

#include <QWidget>
#include <ui/c_widget.h>
#include <utils/c_recipe.h>
#include <ui/c_milestoneview.h>
#include <QVBoxLayout>

namespace Ui {
class c_recipeView;
}

class c_recipeView : public c_widget
{
    Q_OBJECT

public:
    explicit c_recipeView(c_recipe *recipe, c_widget *widget = nullptr, QWidget *parent = nullptr);
    ~c_recipeView() override;

    QAbstractAnimation *switchMode(modes target = modes::resume, bool animated = true,int time = 500, QAnimationGroup *parentGroupAnimation = nullptr) override;
    void handleChildrenAnimation(QAbstractAnimation* animation) override;

    int getImageAreaWidth(modes target) const override;
    int getComponentsAreaWidth(modes target) const override;
    int getProcessesAreaWidth(modes target) const override;

    void resizeEvent(QResizeEvent* e) override;
    void updateOneComponentsList(QAnimationGroup *parentGroupAnimation);

private slots:
    void slotComponentListChanged();
    void slotProcessMapChanged();

private:
    Ui::c_recipeView *ui;
    c_recipe *recipe;
    QList<c_milestoneView *> milestonesViews;
    int heightMilstones;
    c_image *imageRecipe;
    bool componentChanged;
    c_milestoneView *senderComponentChanged;
    c_processView *globalProcessingView;

    QMap<c_milestoneView *,c_componentView*> componentMap;
    QMap<QString,c_process*> processMap;
};

#endif // C_RECIPEVIEW_H
