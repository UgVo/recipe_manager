#ifndef C_PROCESSVIEW_H
#define C_PROCESSVIEW_H

#include <QWidget>
#include <QList>
#include <utils/c_process.h>
#include <ui/c_processelemview.h>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <ui/c_widget.h>

namespace Ui {
class c_processView;
}

class c_processView : public c_widget
{
    Q_OBJECT

public:
    explicit c_processView(QList<c_process *> processes, c_widget *widget, QWidget *parent = nullptr);
    ~c_processView() override;

    QAbstractAnimation *switchMode(modes mode = modes::resume, bool animated = true, int time = 1000) override;
    QSize getSize(modes target) const override;
    int getWidth(modes target) const override;

    void save() override;
    void rollback() override;

    void setProcessings(QList<c_process *> processes);
    c_process *newProcessing();

    bool isEmpty() const;

private:
    Ui::c_processView *ui;
    QList<c_process *> processes;
    QList<c_process *> processSave;
    QList<c_processElemView *> processElems;

    static int maxNumberProcess;
};

#endif // C_PROCESSVIEW_H
