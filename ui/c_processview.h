#ifndef C_PROCESSVIEW_H
#define C_PROCESSVIEW_H

#include <QWidget>
#include <QList>
#include <utils/c_process.h>
#include <ui/c_processelemview.h>
#include <QPropertyAnimation>

namespace Ui {
class c_processView;
}

class c_processView : public QWidget
{
    Q_OBJECT

public:
    explicit c_processView(QList<c_process *> processes, QWidget *parent = nullptr);
    ~c_processView();

    QList<QPropertyAnimation *> switchMode(int mode = recipe::modes::resume, bool animated = true, int time = 1000);
    QSize getSize(int mode);

    bool isEmpty();

private:
    Ui::c_processView *ui;
    QList<c_process *> processes;
    QList<c_processElemView *> processViews;

    static int maxNumberProcess;
};

#endif // C_PROCESSVIEW_H
