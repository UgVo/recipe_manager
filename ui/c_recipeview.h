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

    void resizeEvent(QResizeEvent* e) override;

private:
    Ui::c_recipeView *ui;
    c_recipe *recipe;
    QList<c_milestoneView *> milestonesViews;
    int heightMilstones;
};

#endif // C_RECIPEVIEW_H
