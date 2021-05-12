#ifndef C_MILESTONEVIEW_H
#define C_MILESTONEVIEW_H

#include <ui/c_widget.h>
#include <utils/c_milestone.h>
#include <ui/c_stepview.h>

namespace Ui {
class c_milestoneView;
}

class c_milestoneView : public c_widget
{
    Q_OBJECT

public:
    explicit c_milestoneView(c_milestone *milestone, QWidget *parent = nullptr);
    ~c_milestoneView();

    QAbstractAnimation *switchMode(modes target = modes::resume, bool animated = true,int time = 500, QAbstractAnimation *childAnims = nullptr);
    QSize getSize(modes target) const;
    QSize getStepDisplaySize() const;

public slots:
    void slotHandleResizeStep(QAbstractAnimation *animation);
    void slotSwapSteps(recipe::swap direction);
    void slotDeleteSteps();

private:
    Ui::c_milestoneView *ui;
    c_milestone *milestone;
    QList<c_stepView *> stepList;
};

#endif // C_MILESTONEVIEW_H
