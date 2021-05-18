#ifndef C_MILESTONEVIEW_H
#define C_MILESTONEVIEW_H

#include <ui/c_widget.h>
#include <utils/c_milestone.h>
#include <ui/c_stepview.h>
#include <QStyle>

namespace Ui {
class c_milestoneView;
}

class c_milestoneView : public c_widget
{
    Q_OBJECT

public:
    explicit c_milestoneView(c_milestone *milestone, c_widget *widget = nullptr, QWidget *parent = nullptr);
    ~c_milestoneView() override;

    QAbstractAnimation *switchMode(modes target = modes::resume, bool animated = true,int time = 500, QAbstractAnimation *childAnims = nullptr) override;
    QSize getSize(modes target) const override;
    QSize getStepDisplaySize() const;

    QList<c_process *> getProcessesPtr();

    void setDefaultMode(modes defaultMode, bool animated);
    modes getDefaultMode() const;

public slots:
    void slotHandleResizeStep(QAbstractAnimation *animation);
    void slotSwapSteps(recipe::swap direction);
    void slotDeleteSteps();
    void slotUpdateProcesses();
    void slotAddStep();
    void slotUpdateCurrentCharCount();

private:
    Ui::c_milestoneView *ui;
    c_milestone *milestone;
    QList<c_stepView *> stepList;
    QMap<QString,c_process> processMap;
    c_processView *processResume;
    QLabel *arrow;
    QPixmap arrowPixmapUp;
    QPixmap arrowPixmapDown;

    modes defaultMode;
};

#endif // C_MILESTONEVIEW_H
