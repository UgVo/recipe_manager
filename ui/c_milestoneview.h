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

    const QMap<QString, c_component *> &getComponents() const;
    QList<c_component *> getComponentsList();
    void setComponentsList(const QMap<QString, c_component*> &newComponentsList);

    c_milestone *getMilestone() const;

    const QMap<QString, c_process> &getProcessMap() const;
    void setProcessMap(const QMap<QString, c_process> &newProcessMap);

public slots:
    void slotHandleResizeStep();
    void slotSwapSteps(recipe::swap direction);
    void slotDeleteSteps();
    void slotUpdateProcesses();
    void slotAddStep();
    void slotUpdateCurrentCharCount();
    void slotUpdateComponentsList();

signals:
    void resized();
    void componentsListChanged();
    void processMapChanged();

private:
    Ui::c_milestoneView *ui;
    c_milestone *milestone;
    QList<c_stepView *> stepList;
    QMap<QString,c_process> processMap;
    c_processView *processResume;

    QGraphicsScene* scene;
    QGraphicsView* view;
    c_pixmapGraphics* item;

    QMap<QString,c_component *> componentsList;

    modes defaultMode;
    Q_PROPERTY(QMap<QString, c_component *> componentsList READ getComponents WRITE setComponentsList NOTIFY componentsListChanged)
    Q_PROPERTY(QMap<QString, c_process> processMap READ getProcessMap WRITE setProcessMap NOTIFY processMapChanged)
};

#endif // C_MILESTONEVIEW_H
