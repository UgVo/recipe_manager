#ifndef C_MILESTONEVIEW_H
#define C_MILESTONEVIEW_H

#include <ui/c_widget.h>
#include <utils/c_milestone.h>
#include <ui/c_stepview.h>
#include <QStyle>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>

namespace Ui {
class c_milestoneView;
}

class c_milestoneView : public c_widget
{
    Q_OBJECT

public:
    explicit c_milestoneView(c_milestone *milestone, c_widget *widget = nullptr, QWidget *parent = nullptr);
    ~c_milestoneView() override;

    QAbstractAnimation *switchMode(modes target = modes::resume, bool animated = true,int time = 500, QAnimationGroup *parentGroupAnimation = nullptr) override;
    QSize getSize(modes target = modes::none) const override;
    void resizeEvent(QResizeEvent *event) override;
    QSize getStepDisplaySize() const;

    QList<c_process *> getProcessesPtr();

    void setDefaultMode(modes defaultMode, bool animated);
    modes getDefaultMode() const;

    bool eventFilter(QObject *obj,QEvent *event) override;
    void handleChildrenAnimation(QAbstractAnimation *animation) override;

public slots:
    void slotHandleResizeStep();
    void slotSwapSteps(recipe::swap direction);
    void slotDeleteSteps();
    void slotUpdateProcesses();
    void slotAddStep();
    void slotUpdateCurrentCharCount();

signals:
    void resized();

private:
    Ui::c_milestoneView *ui;
    c_milestone *milestone;
    QList<c_stepView *> stepList;
    QMap<QString,c_process> processMap;
    c_processView *processResume;

    QGraphicsScene* scene;
    QGraphicsView* view;
    c_pixmapGraphics* item;

    modes defaultMode;
};

#endif // C_MILESTONEVIEW_H
