#ifndef C_PROCESSELEMVIEW_H
#define C_PROCESSELEMVIEW_H

#include <QWidget>
#include <QStringListModel>
#include <QLineEdit>
#include <QPropertyAnimation>
#include "utils/c_process.h"
#include "utils/c_dbmanager.h"
#include <QFontMetrics>
#include <ui/c_widget.h>


namespace Ui {
class c_processElemView;
}

class c_processElemView : public c_widget
{
    Q_OBJECT

public:
    explicit c_processElemView(c_process* process =  nullptr, QWidget *parent = nullptr);
    ~c_processElemView();

    QAnimationGroup *switchMode(int target = recipe::modes::resume, bool animated = true,int time = 1000);
    QSize getSize(int target);
    void save();
    void rollback();

    bool isEmpty();

    static int heightProcess;

    void setProcess(c_process *value);

signals:
    void removeProcess(const c_process *);

private:
    Ui::c_processElemView *ui;
    c_process* process;
};

#endif // C_PROCESSELEMVIEW_H
